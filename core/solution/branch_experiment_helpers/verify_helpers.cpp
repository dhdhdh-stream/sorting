#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "state_network.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	double original_predicted_score = this->existing_average_score;
	double branch_predicted_score = this->new_average_score;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->existing_input_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_input_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->new_input_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_input_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->existing_local_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_local_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->new_local_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_local_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<State*, double>::iterator original_weight_it = this->existing_temp_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_temp_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<State*, double>::iterator branch_weight_it = this->new_temp_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_temp_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch;
	if (abs(branch_predicted_score - original_predicted_score) > DECISION_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
		decision_is_branch = branch_predicted_score > original_predicted_score;
	} else {
		uniform_int_distribution<int> distribution(0, 1);
		decision_is_branch = distribution(generator);
	}
	#endif /* MDEBUG */

	this->branch_possible++;
	if (decision_is_branch) {
		this->branch_count++;

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
				this->best_actions[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					action_node_history);
				delete action_node_history;
			} else {
				PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_potential_scopes[s_index]);
				this->best_potential_scopes[s_index]->activate(problem,
															   context,
															   run_helper,
															   potential_scope_node_history);
				delete potential_scope_node_history;
			}
		}

		if (this->best_is_exit) {
			run_helper.has_exited = true;
		} else {
			if (this->best_exit_depth == 0) {
				curr_node = this->best_exit_node;
			} else {
				curr_node = NULL;

				exit_depth = this->best_exit_depth-1;
				exit_node = this->best_exit_node;
			}
		}
	}
}

void BranchExperiment::verify_backprop(double target_val,
									   RunHelper& run_helper) {
	this->combined_score += target_val;

	#if defined(MDEBUG) && MDEBUG
	if (run_helper.exceeded_limit) {
		this->new_exceeded_limit = true;
	}
	#endif /* MDEBUG */

	this->state_iter++;
	if (this->state == BRANCH_EXPERIMENT_STATE_VERIFY_1ST
			&& this->state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		this->combined_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		#if defined(MDEBUG) && MDEBUG
		if (!this->new_exceeded_limit && rand()%2 == 0) {
		#else
		double score_standard_deviation = sqrt(this->verify_existing_score_variance);
		double combined_improvement = this->combined_score - this->verify_existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

		double branch_weight = (double)this->branch_count / (double)this->branch_possible;

		if (branch_weight > 0.01 && combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			this->combined_score = 0.0;
			this->branch_count = 0;
			this->branch_possible = 0;

			this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			this->state = BRANCH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		} else {
			cout << "Branch verify 1st fail" << endl;
			this->state = BRANCH_EXPERIMENT_STATE_FAIL;
		}
	} else if (this->state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		this->combined_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		double score_standard_deviation = sqrt(this->verify_existing_score_variance);
		double combined_improvement = this->combined_score - this->verify_existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

		double branch_weight = (double)this->branch_count / (double)this->branch_possible;

		#if defined(MDEBUG) && MDEBUG
		if (!this->new_exceeded_limit && rand()%2 == 0) {
		#else
		if (branch_weight > 0.01 && combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			cout << "Branch" << endl;
			cout << "verify" << endl;
			cout << "this->scope_context:" << endl;
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				cout << c_index << ": " << this->scope_context[c_index] << endl;
			}
			cout << "this->node_context:" << endl;
			for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
				cout << c_index << ": " << this->node_context[c_index] << endl;
			}
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else {
					cout << " S";
				}
			}
			cout << endl;

			cout << "this->best_is_exit: " << this->best_is_exit << endl;
			cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
			if (this->best_exit_node == NULL) {
				cout << "this->best_exit_node_id: " << -1 << endl;
			} else {
				cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
			}

			cout << "this->combined_score: " << this->combined_score << endl;
			cout << "this->verify_existing_average_score: " << this->verify_existing_average_score << endl;
			cout << "score_standard_deviation: " << score_standard_deviation << endl;
			cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

			cout << "branch_weight: " << branch_weight << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (branch_weight > 0.99
					&& this->new_average_score > this->existing_average_score) {
			#endif /* MDEBUG */
				this->is_pass_through = true;
			} else {
				this->is_pass_through = false;
			}

			#if defined(MDEBUG) && MDEBUG
			this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
			this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

			/**
			 * - if parent_pass_through_experiment == NULL, no earlier PotentialScopeNodes for new state to be added in
			 * 
			 * - if is_pass_through, no new state will be added
			 */
			if (this->parent_pass_through_experiment != NULL && !this->is_pass_through) {
				set<int> needed_state;
				set<int> needed_parent_state;
				for (map<State*, double>::iterator it = this->existing_temp_state_weights[0].begin();
						it != this->existing_temp_state_weights[0].end(); it++) {
					for (int ns_index = 0; ns_index < (int)this->new_states.size(); ns_index++) {
						if (this->new_states[ns_index] == it->first) {
							needed_state.insert(ns_index);
							break;
						}
					}
					for (int ns_index = 0; ns_index < (int)this->parent_pass_through_experiment->new_states.size(); ns_index++) {
						if (this->parent_pass_through_experiment->new_states[ns_index] == it->first) {
							needed_parent_state.insert(ns_index);
							break;
						}
					}
				}

				for (set<int>::iterator it = needed_state.begin(); it != needed_state.end(); it++) {
					for (int n_index = 0; n_index < (int)this->new_state_nodes[*it].size(); n_index++) {
						bool matches_scope = true;
						if (this->new_state_scope_contexts[*it][n_index].size() < this->scope_context.size()) {
							matches_scope = false;
						} else {
							for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
								if (this->new_state_scope_contexts[*it][n_index][c_index] != this->scope_context[c_index]) {
									matches_scope = false;
									break;
								}
							}
							for (int c_index = 0; c_index < (int)this->node_context.size()-1; c_index++) {
								if (this->new_state_node_contexts[*it][n_index][c_index] != this->node_context[c_index]) {
									matches_scope = false;
									break;
								}
							}
						}

						if (matches_scope) {
							PotentialScopeNode* potential_scope_node = NULL;
							for (int s_index = 0; s_index < (int)this->parent_pass_through_experiment->best_step_types.size(); s_index++) {
								if (this->parent_pass_through_experiment->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
									if (this->parent_pass_through_experiment->best_potential_scopes[s_index]->scope_node_placeholder->id
											== this->new_state_node_contexts[*it][n_index][this->scope_context.size()-1]) {
										potential_scope_node = this->parent_pass_through_experiment->best_potential_scopes[s_index];
										break;
									}
								}
							}

							if (potential_scope_node != NULL) {
								potential_scope_node->used_experiment_states.insert(this->new_states[*it]);
							}
						}
					}
				}

				for (set<int>::iterator it = needed_parent_state.begin(); it != needed_parent_state.end(); it++) {
					for (int n_index = 0; n_index < (int)this->parent_pass_through_experiment->new_state_nodes[*it].size(); n_index++) {
						bool matches_scope = true;
						if (this->parent_pass_through_experiment->new_state_scope_contexts[*it][n_index].size() < this->scope_context.size()) {
							matches_scope = false;
						} else {
							for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
								if (this->parent_pass_through_experiment->new_state_scope_contexts[*it][n_index][c_index] != this->scope_context[c_index]) {
									matches_scope = false;
									break;
								}
							}
							for (int c_index = 0; c_index < (int)this->node_context.size()-1; c_index++) {
								if (this->parent_pass_through_experiment->new_state_node_contexts[*it][n_index][c_index] != this->node_context[c_index]) {
									matches_scope = false;
									break;
								}
							}
						}

						if (matches_scope) {
							PotentialScopeNode* potential_scope_node = NULL;
							for (int s_index = 0; s_index < (int)this->parent_pass_through_experiment->best_step_types.size(); s_index++) {
								if (this->parent_pass_through_experiment->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
									if (this->parent_pass_through_experiment->best_potential_scopes[s_index]->scope_node_placeholder->id
											== this->parent_pass_through_experiment->new_state_node_contexts[*it][n_index][this->scope_context.size()-1]) {
										potential_scope_node = this->parent_pass_through_experiment->best_potential_scopes[s_index];
										break;
									}
								}
							}

							if (potential_scope_node != NULL) {
								potential_scope_node->used_experiment_states.insert(this->parent_pass_through_experiment->new_states[*it]);
							}
						}
					}
				}
			}

			this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
			this->state_iter = 0;
			#else
			this->state = BRANCH_EXPERIMENT_STATE_SUCCESS;
			#endif /* MDEBUG */
		} else {
			cout << "Branch verify 2nd fail" << endl;
			this->state = BRANCH_EXPERIMENT_STATE_FAIL;
		}
	}
}
