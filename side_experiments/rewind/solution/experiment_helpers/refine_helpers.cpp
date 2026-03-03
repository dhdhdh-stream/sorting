#include "experiment.h"

#include <algorithm>
#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void Experiment::refine_check_activate(SolutionWrapper* wrapper) {
	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::refine_step(vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	ScopeHistory* scope_history = wrapper->scope_histories.back();

	if (experiment_state->step_index == 0) {
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

		if (!is_branch) {
			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
			return;
		} else {
			ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

			this->new_obs_histories.push_back(obs);

			this->existing_true_network->activate(obs);
			history->existing_predicted_trues.push_back(
				this->existing_true_network->output->acti_vals[0]);
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
			ScopeNode* scope_node = (ScopeNode*)this->best_new_nodes[experiment_state->step_index];

			ScopeNodeHistory* history = new ScopeNodeHistory(scope_node);
			scope_history->node_histories[scope_node->id] = history;

			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			history->scope_history = inner_scope_history;
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::refine_exit_step(SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::refine_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	this->total_count++;
	this->total_sum_scores += target_val;

	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->existing_predicted_trues.size() > 0) {
		this->sum_true += target_val;
		this->hit_count++;

		for (int i_index = 0; i_index < (int)history->existing_predicted_trues.size(); i_index++) {
			this->new_true_histories.push_back(target_val - history->existing_predicted_trues[i_index]);
		}

		this->state_iter++;
		if (this->state_iter >= MEASURE_ITERS) {
			bool is_success = false;
			if (this->new_networks.size() == 0) {
				double new_true = this->sum_true / this->hit_count;
				this->local_improvement = new_true - this->existing_true;
				double average_hits_per_run = (double)this->hit_count / (double)this->total_count;
				this->global_improvement = average_hits_per_run * this->local_improvement;

				if (this->local_improvement >= 0.0) {
					if (wrapper->solution->last_passthrough_scores.size() >= MIN_NUM_LAST_TRACK) {
						int num_better_than = 0;
						for (list<double>::iterator it = wrapper->solution->last_passthrough_scores.begin();
								it != wrapper->solution->last_passthrough_scores.end(); it++) {
							if (this->local_improvement >= *it) {
								num_better_than++;
							}
						}

						int target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->last_passthrough_scores.size();

						if (num_better_than >= target_better_than) {
							is_success = true;
						}

						if (wrapper->solution->last_passthrough_scores.size() >= NUM_LAST_TRACK) {
							wrapper->solution->last_passthrough_scores.pop_front();
						}
						wrapper->solution->last_passthrough_scores.push_back(this->local_improvement);
					} else {
						wrapper->solution->last_passthrough_scores.push_back(this->local_improvement);
					}
				}
			}

			if (is_success) {
				this->best_refine_is_binarize = false;

				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				if (this->best_new_scope != NULL) {
					if (this->best_parent_scope == this->scope_context) {
						cout << "new local scope" << endl;
					} else {
						cout << "new outer scope" << endl;
					}
				}
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index];
					} else {
						if (this->best_scopes[s_index]->is_outer) {
							cout << " O" << this->best_scopes[s_index]->id;
						} else {
							cout << " E" << this->best_scopes[s_index]->id;
						}
					}
				}
				cout << endl;

				if (this->exit_next_node == NULL) {
					cout << "this->exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
				}

				cout << "this->local_improvement: " << this->local_improvement << endl;
				cout << "this->global_improvement: " << this->global_improvement << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
				}
				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_true_histories.begin(), this->new_true_histories.end(), generator_copy);
				}

				double best_val_average = numeric_limits<double>::lowest();
				Network* new_network = NULL;
				train_and_eval_helper(best_val_average,
									  new_network,
									  this->best_refine_is_binarize);

				this->new_obs_histories.clear();
				this->new_true_histories.clear();

				if (new_network != NULL) {
					this->new_networks.push_back(new_network);

					this->sum_true = 0.0;
					this->hit_count = 0;

					this->total_count = 0;
					this->total_sum_scores = 0.0;

					this->state = EXPERIMENT_STATE_MEASURE;
					this->state_iter = 0;
				} else {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			}
		}
	}
}
