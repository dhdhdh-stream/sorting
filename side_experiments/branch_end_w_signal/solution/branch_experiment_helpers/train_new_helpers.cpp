#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_DATAPOINTS = 20;
#else
const int TRAIN_NEW_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

const double MIN_CONSISTENCY_RATIO = 0.2;

void BranchExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	this->num_instances_until_target--;
	if (this->num_instances_until_target <= 0) {
		uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
		this->num_instances_until_target = until_distribution(generator);

		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void BranchExperiment::train_new_step(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;

		this->existing_network->activate(obs);
		history->existing_predicted_scores.push_back(this->existing_network->output->acti_vals[0]);

		this->obs_histories.push_back(obs);

		history->stack_traces.push_back(wrapper->scope_histories);
	}

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

void BranchExperiment::train_new_exit_step(SolutionWrapper* wrapper,
										   BranchExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::train_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		map<Scope*, pair<int,ScopeHistory*>> to_add;
		for (int s_index = 0; s_index < (int)history->stack_traces.size(); s_index++) {
			bool is_consistent = true;
			double sum_vals = target_val - wrapper->solution->curr_score;
			int sum_counts = 1;

			for (int l_index = 0; l_index < (int)history->stack_traces[s_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[s_index][l_index];
				Scope* scope = scope_history->scope;

				if (scope->consistency_network != NULL) {
					if (!scope_history->signal_initialized) {
						vector<double> inputs = scope_history->pre_obs;
						inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

						scope->consistency_network->activate(inputs);
						scope_history->signal_initialized = true;
						#if defined(MDEBUG) && MDEBUG
						scope_history->consistency_val = 2 * (rand()%2) - 1;
						#else
						scope_history->consistency_val = scope->consistency_network->output->acti_vals[0];
						#endif /* MDEBUG */

						if (scope_history->consistency_val >= CONSISTENCY_MATCH_WEIGHT) {
							scope->pre_network->activate(scope_history->pre_obs);
							scope_history->pre_val = scope->pre_network->output->acti_vals[0];

							scope->post_network->activate(inputs);
							scope_history->post_val = scope->post_network->output->acti_vals[0];
						}
					}

					if (scope_history->consistency_val < CONSISTENCY_MATCH_WEIGHT) {
						is_consistent = false;
						break;
					} else {
						sum_vals += (scope_history->post_val - scope_history->pre_val);
						sum_counts++;
					}
				}

				if (scope->signal_status != SIGNAL_STATUS_FAIL) {
					map<Scope*, pair<int,ScopeHistory*>>::iterator it = to_add.find(scope);
					if (it == to_add.end()) {
						to_add[scope] = {1, scope_history};
					} else {
						uniform_int_distribution<int> add_distribution(0, it->second.first);
						if (add_distribution(generator) == 0) {
							it->second.second = scope_history;
						}
						it->second.first++;
					}
				}
			}

			double average_val = sum_vals / sum_counts;

			this->consistency_histories.push_back(is_consistent);
			this->target_val_histories.push_back(average_val - history->existing_predicted_scores[s_index]);
		}

		for (map<Scope*, pair<int,ScopeHistory*>>::iterator it = to_add.begin();
				it != to_add.end(); it++) {
			Scope* scope = it->first;
			ScopeHistory* scope_history = it->second.second;
			int max_sample_per_timestamp = (TOTAL_MAX_SAMPLES + (int)scope->existing_pre_obs.size() - 1) / (int)scope->existing_pre_obs.size();
			if ((int)scope->explore_pre_obs.back().size() < max_sample_per_timestamp) {
				scope->explore_pre_obs.back().push_back(scope_history->pre_obs);
				scope->explore_post_obs.back().push_back(scope_history->post_obs);
			} else {
				uniform_int_distribution<int> distribution(0, scope->explore_pre_obs.back().size()-1);
				int index = distribution(generator);
				scope->explore_pre_obs.back()[index] = scope_history->pre_obs;
				scope->explore_post_obs.back()[index] = scope_history->post_obs;
			}
		}

		this->state_iter++;
		if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS
				&& (int)this->target_val_histories.size() >= TRAIN_NEW_NUM_DATAPOINTS) {
			int num_consistent = 0;
			for (int h_index = 0; h_index < (int)this->consistency_histories.size(); h_index++) {
				if (this->consistency_histories[h_index]) {
					num_consistent++;
				}
			}

			if (num_consistent < MIN_CONSISTENCY_RATIO * (double)this->consistency_histories.size()) {
				this->result = EXPERIMENT_RESULT_FAIL;
				return;
			}

			if (num_consistent != (int)this->consistency_histories.size()) {
				this->new_consistency_network = new Network(this->obs_histories[0].size(),
															NETWORK_SIZE_SMALL);
				uniform_int_distribution<int> consistency_input_distribution(0, this->obs_histories.size()-1);
				for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
					int rand_index = consistency_input_distribution(generator);

					this->new_consistency_network->activate(this->obs_histories[rand_index]);

					double error;
					if (this->consistency_histories[rand_index]) {
						if (this->new_consistency_network->output->acti_vals[0] >= 1.0) {
							error = 0.0;
						} else {
							error = 1.0 - this->new_consistency_network->output->acti_vals[0];
						}
					} else {
						if (this->new_consistency_network->output->acti_vals[0] <= -1.0) {
							error = 0.0;
						} else {
							error = -1.0 - this->new_consistency_network->output->acti_vals[0];
						}
					}

					this->new_consistency_network->backprop(error);
				}
			}

			this->new_val_network = new Network(this->obs_histories[0].size(),
												NETWORK_SIZE_SMALL);
			vector<vector<double>> consistent_obs_histories;
			vector<double> consistent_target_val_histories;
			for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
				if (this->consistency_histories[h_index]) {
					consistent_obs_histories.push_back(this->obs_histories[h_index]);
					consistent_target_val_histories.push_back(this->target_val_histories[h_index]);
				}
			}
			uniform_int_distribution<int> val_input_distribution(0, consistent_obs_histories.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->new_val_network->activate(consistent_obs_histories[rand_index]);

				double error = consistent_target_val_histories[rand_index] - this->new_val_network->output->acti_vals[0];

				this->new_val_network->backprop(error);
			}

			vector<double> network_outputs(consistent_obs_histories.size());
			for (int h_index = 0; h_index < (int)consistent_obs_histories.size(); h_index++) {
				this->new_val_network->activate(consistent_obs_histories[h_index]);

				network_outputs[h_index] = this->new_val_network->output->acti_vals[0];
			}

			int num_positive = 0;
			for (int i_index = 0; i_index < (int)consistent_obs_histories.size(); i_index++) {
				if (network_outputs[i_index] >= 0.0) {
					num_positive++;
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if (num_positive > 0 || rand()%4 != 0) {
			#else
			if (num_positive > 0) {
			#endif /* MDEBUG */
				this->total_count = 0;
				this->total_sum_scores = 0.0;

				this->sum_scores = 0.0;

				this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
