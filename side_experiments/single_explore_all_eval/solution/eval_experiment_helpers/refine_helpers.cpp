#include "eval_experiment.h"

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 5000;
#endif /* MDEBUG */

const double VERIFY_RATIO = 0.2;

void EvalExperiment::refine_check_activate(
		SolutionWrapper* wrapper) {
	if (wrapper->curr_explore_experiment == NULL
			&& wrapper->damage_state == DAMAGE_STATE_REFINE) {
		EvalExperimentState* new_experiment_state = new EvalExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void EvalExperiment::refine_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper) {
	EvalExperimentState* experiment_state = (EvalExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		EvalExperimentHistory* history;
		map<EvalExperiment*, EvalExperimentHistory*>::iterator it =
			wrapper->eval_experiment_histories.find(this);
		if (it == wrapper->eval_experiment_histories.end()) {
			history = new EvalExperimentHistory(this);
			wrapper->eval_experiment_histories[this] = history;
		} else {
			history = it->second;
		}

		bool is_branch = true;
		for (int n_index = 0; n_index < (int)this->new_networks.size(); n_index++) {
			this->new_networks[n_index]->activate(obs);
			if (this->new_networks[n_index]->output->acti_vals[0] < 0.0) {
				is_branch = false;
				break;
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#endif /* MDEBUG */

		if (is_branch) {
			history->hit_branch = true;

			if (history->is_on) {
				this->new_obs_histories.push_back(obs);
			} else {
				if (this->existing_obs_histories.size() < 2 * this->new_obs_histories.size()) {
					this->existing_obs_histories.push_back(obs);
				}

				delete experiment_state;
				wrapper->experiment_context.back() = NULL;
				return;
			}
		} else {
			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
			return;
		}
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void EvalExperiment::refine_exit_step(SolutionWrapper* wrapper) {
	EvalExperimentState* experiment_state = (EvalExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void EvalExperiment::refine_backprop(double target_val,
									 EvalExperimentHistory* history,
									 SolutionWrapper* wrapper) {
	if (history->is_on) {
		if (history->hit_branch) {
			while (this->new_true_histories.size() < this->new_obs_histories.size()) {
				this->new_true_histories.push_back(target_val);
			}

			this->state_iter++;
		}
	} else {
		if (history->hit_branch) {
			while (this->existing_true_histories.size() < this->existing_obs_histories.size()) {
				this->existing_true_histories.push_back(target_val);
			}
		}
	}

	if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
		Network* existing_network = new Network(this->existing_obs_histories[0].size(),
												NETWORK_SIZE_SMALL);

		uniform_int_distribution<int> existing_distribution(0, this->existing_obs_histories.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = existing_distribution(generator);

			existing_network->activate(this->existing_obs_histories[rand_index]);

			double error = this->existing_true_histories[rand_index] - existing_network->output->acti_vals[0];

			existing_network->backprop(error);
		}
		for (int h_index = 0; h_index < (int)this->new_obs_histories.size(); h_index++) {
			existing_network->activate(this->new_obs_histories[h_index]);
			this->new_true_histories[h_index] -= existing_network->output->acti_vals[0];
		}
		this->existing_obs_histories.clear();
		this->existing_true_histories.clear();
		delete existing_network;

		Network* new_network = new Network(this->new_obs_histories[0].size(),
										   NETWORK_SIZE_SMALL);

		int num_train = (1.0 - VERIFY_RATIO) * (double)this->new_obs_histories.size();

		uniform_int_distribution<int> distribution(0, num_train-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int rand_index = distribution(generator);

			new_network->activate(this->new_obs_histories[rand_index]);

			double error = this->new_true_histories[rand_index] - new_network->output->acti_vals[0];

			new_network->backprop(error);
		}

		double sum_vals = 0.0;
		for (int h_index = num_train; h_index < (int)this->new_obs_histories.size(); h_index++) {
			new_network->activate(this->new_obs_histories[h_index]);

			if (new_network->output->acti_vals[0] >= 0.0) {
				sum_vals += this->new_true_histories[h_index];
			}
		}
		double local_improvement = sum_vals / ((double)this->new_obs_histories.size() - (double)num_train);

		int total_iters = wrapper->eval_iter - this->starting_iter;
		if (total_iters < 0) {
			total_iters += numeric_limits<int>::max();
		}
		double average_hits_per_run = ((double)this->existing_scores.size() + (double)this->new_scores.size()) / (double)total_iters;

		double global_improvement = average_hits_per_run * local_improvement;

		bool is_success = false;
		if (local_improvement > 0.0) {
			if (wrapper->solution->train_new_last_scores.size() >= MIN_NUM_LAST_TRACK) {
				int num_better_than = 0;
				for (list<double>::iterator it = wrapper->solution->train_new_last_scores.begin();
						it != wrapper->solution->train_new_last_scores.end(); it++) {
					if (global_improvement >= *it) {
						num_better_than++;
					}
				}

				double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->train_new_last_scores.size();

				if (num_better_than >= target_better_than) {
					is_success = true;
				}

				if (wrapper->solution->train_new_last_scores.size() >= NUM_LAST_TRACK) {
					wrapper->solution->train_new_last_scores.pop_front();
				}
				wrapper->solution->train_new_last_scores.push_back(global_improvement);
			} else {
				wrapper->solution->train_new_last_scores.push_back(global_improvement);
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (is_success || rand()%3 != 0) {
		#else
		if (is_success) {
		#endif /* MDEBUG */
			this->new_networks.push_back(new_network);

			this->new_obs_histories.clear();
			this->new_true_histories.clear();

			this->curr_ramp = 0;
			this->measure_status = MEASURE_STATUS_N_A;

			this->state = EVAL_EXPERIMENT_STATE_RAMP;
			this->state_iter = 0;
		} else {
			delete new_network;

			this->node_context->experiment = NULL;
			delete this;
		}
	}
}
