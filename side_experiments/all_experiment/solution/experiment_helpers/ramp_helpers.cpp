#include "experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void Experiment::ramp_check_activate(
		vector<double>& obs,
		SolutionWrapper* wrapper,
		ExperimentHistory* history) {
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
			ExperimentState* new_experiment_state = new ExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void Experiment::ramp_step(vector<double>& obs,
						   int& action,
						   bool& is_next,
						   SolutionWrapper* wrapper,
						   ExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

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

void Experiment::ramp_exit_step(SolutionWrapper* wrapper,
								ExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::ramp_backprop(double target_val,
							   ExperimentHistory* history,
							   SolutionWrapper* wrapper,
							   set<Scope*>& updated_scopes) {
	if (history->is_on) {
		if (history->hit_branch) {
			this->new_sum_scores += target_val;
			this->new_count++;

			this->state_iter++;
		}
	} else {
		if (history->hit_branch) {
			this->existing_sum_scores += target_val;
			this->existing_count++;

			this->state_iter++;
		}
	}

	switch (this->state) {
	case EXPERIMENT_STATE_RAMP:
		if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
			double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
			double new_score_average = this->new_sum_scores / (double)this->new_count;

			// // temp
			// cout << "this->curr_ramp: " << this->curr_ramp << endl;
			// cout << "existing_score_average: " << existing_score_average << endl;
			// cout << "new_score_average: " << new_score_average << endl;

			this->existing_sum_scores = 0.0;
			this->existing_count = 0;
			this->new_sum_scores = 0.0;
			this->new_count = 0;

			this->state_iter = 0;

			#if defined(MDEBUG) && MDEBUG
			if ((this->measure_status != MEASURE_STATUS_FAIL && new_score_average > existing_score_average) || rand()%3 != 0) {
			#else
			if (this->measure_status != MEASURE_STATUS_FAIL && new_score_average > existing_score_average) {
			#endif /* MDEBUG */
				this->curr_ramp++;

				if (this->curr_ramp == MEASURE_GEAR
						&& this->measure_status == MEASURE_STATUS_N_A) {
					this->starting_iter = wrapper->iter;

					this->state = EXPERIMENT_STATE_MEASURE;
				} else if (this->curr_ramp == EXPERIMENT_NUM_GEARS) {
					updated_scopes.insert(this->node_context->parent);

					add(wrapper);
					this->node_context->experiment = NULL;
					delete this;
				}
			} else {
				this->curr_ramp--;
				if (this->curr_ramp < 0) {
					this->node_context->experiment = NULL;
					delete this;
				}
			}
		}
		break;
	case EXPERIMENT_STATE_MEASURE:
		if (this->state_iter >= MEASURE_STEP_NUM_ITERS) {
			double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
			double new_score_average = this->new_sum_scores / (double)this->new_count;

			int total_iters = wrapper->iter - this->starting_iter;
			if (total_iters < 0) {
				total_iters += numeric_limits<int>::max();
			}
			double average_hits_per_run = (2.0 * (double)this->new_count) / (double)total_iters;

			this->local_improvement = new_score_average - existing_score_average;
			this->global_improvement = average_hits_per_run * this->local_improvement;

			// // temp
			// cout << "new_score_average: " << new_score_average << endl;
			// cout << "existing_score_average: " << existing_score_average << endl;
			// cout << "this->local_improvement: " << this->local_improvement << endl;
			// cout << "this->global_improvement: " << this->global_improvement << endl;

			#if defined(MDEBUG) && MDEBUG
			if (this->local_improvement > 0.0 || rand()%3 == 0) {
			#else
			if (this->local_improvement > 0.0) {
			#endif /* MDEBUG */
				bool is_success = false;
				if (wrapper->solution->ramp_last_scores.size() >= MIN_NUM_LAST_TRACK) {
					int num_better_than = 0;
					for (list<double>::iterator it = wrapper->solution->ramp_last_scores.begin();
							it != wrapper->solution->ramp_last_scores.end(); it++) {
						if (this->global_improvement >= *it) {
							num_better_than++;
						}
					}

					double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->ramp_last_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					}

					if (wrapper->solution->ramp_last_scores.size() >= NUM_LAST_TRACK) {
						wrapper->solution->ramp_last_scores.pop_front();
					}
					wrapper->solution->ramp_last_scores.push_back(this->global_improvement);
				} else {
					wrapper->solution->ramp_last_scores.push_back(this->global_improvement);
				}

				#if defined(MDEBUG) && MDEBUG
				if (is_success || rand()%3 != 0) {
				#else
				if (is_success) {
				#endif /* MDEBUG */
					this->measure_status = MEASURE_STATUS_SUCCESS;

					this->curr_ramp++;
				} else {
					this->measure_status = MEASURE_STATUS_FAIL;

					this->curr_ramp--;
				}
			} else {
				this->measure_status = MEASURE_STATUS_FAIL;

				this->curr_ramp--;
			}

			this->existing_sum_scores = 0.0;
			this->existing_count = 0;
			this->new_sum_scores = 0.0;
			this->new_count = 0;

			this->state = EXPERIMENT_STATE_RAMP;
			this->state_iter = 0;
		}
		break;
	}
}

void Experiment::result_ramp_check_activate(
		vector<double>& obs,
		SolutionWrapper* wrapper) {
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
		ExperimentState* new_experiment_state = new ExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->result_experiment_context.back() = new_experiment_state;
	}
}

void Experiment::result_ramp_step(vector<double>& obs,
								  int& action,
								  bool& is_next,
								  SolutionWrapper* wrapper,
								  ExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->result_node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->result_experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->result_num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->result_scope_histories.push_back(inner_scope_history);
			wrapper->result_node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->result_experiment_context.push_back(NULL);
		}
	}
}

void Experiment::result_ramp_exit_step(SolutionWrapper* wrapper,
									   ExperimentState* experiment_state) {
	delete wrapper->result_scope_histories.back();

	wrapper->result_scope_histories.pop_back();
	wrapper->result_node_context.pop_back();
	wrapper->result_experiment_context.pop_back();

	experiment_state->step_index++;
}
