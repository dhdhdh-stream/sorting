#include "orientation_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

bool OrientationExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	run_helper.num_actions_limit = MAX_NUM_ACTIONS_LIMIT_MULTIPLIER * solution->explore_scope_max_num_actions;

	run_helper.num_decisions++;

	vector<double> existing_input_vals(this->existing_input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->existing_input_scope_contexts.size(); i_index++) {
		int curr_layer = 0;
		ScopeHistory* curr_scope_history = context.back().scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				this->existing_input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)this->existing_input_scope_contexts[i_index].size()-1) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							existing_input_vals[i_index] = action_node_history->obs_snapshot[this->existing_input_obs_indexes[i_index]];
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							if (branch_node_history->is_branch) {
								existing_input_vals[i_index] = 1.0;
							} else {
								existing_input_vals[i_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_SCOPE:
						{
							InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
							if (info_scope_node_history->is_positive) {
								existing_input_vals[i_index] = 1.0;
							} else {
								existing_input_vals[i_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
							if (info_branch_node_history->is_branch) {
								existing_input_vals[i_index] = 1.0;
							} else {
								existing_input_vals[i_index] = -1.0;
							}
						}
						break;
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}
	this->existing_network->activate(existing_input_vals);
	#if defined(MDEBUG) && MDEBUG
	#else
	double existing_predicted_score = this->existing_network->output->acti_vals[0];
	#endif /* MDEBUG */

	vector<double> new_input_vals(this->new_input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
		int curr_layer = 0;
		ScopeHistory* curr_scope_history = context.back().scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				this->new_input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)this->new_input_scope_contexts[i_index].size()-1) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							if (branch_node_history->is_branch) {
								new_input_vals[i_index] = 1.0;
							} else {
								new_input_vals[i_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_SCOPE:
						{
							InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
							if (info_scope_node_history->is_positive) {
								new_input_vals[i_index] = 1.0;
							} else {
								new_input_vals[i_index] = -1.0;
							}
						}
						break;
					case NODE_TYPE_INFO_BRANCH:
						{
							InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
							if (info_branch_node_history->is_branch) {
								new_input_vals[i_index] = 1.0;
							} else {
								new_input_vals[i_index] = -1.0;
							}
						}
						break;
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}
	this->new_network->activate(new_input_vals);
	#if defined(MDEBUG) && MDEBUG
	#else
	double new_predicted_score = this->new_network->output->acti_vals[0];
	#endif /* MDEBUG */

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch = new_predicted_score >= existing_predicted_score;
	#endif /* MDEBUG */

	if (decision_is_branch) {
		this->branch_count++;

		if (this->step_types.size() == 0) {
			curr_node = this->exit_next_node;
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else {
				curr_node = this->scopes[0];
			}
		}

		return true;
	} else {
		this->original_count++;

		return false;
	}
}

void OrientationExperiment::measure_backprop(
		EvalHistory* outer_eval_history,
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (run_helper.num_actions_limit == 0) {
		run_helper.num_actions_limit = -1;

		this->result = EXPERIMENT_RESULT_FAIL;
	} else {
		double target_impact;
		if (context.size() == 1) {
			target_impact = problem->score_result(run_helper.num_decisions);
		} else {
			target_impact = context[context.size()-2].scope->eval->calc_impact(outer_eval_history);
		}

		double combined_predicted_impact = this->eval_context->calc_impact(eval_history);

		double combined_misguess = (target_impact - combined_predicted_impact) * (target_impact - combined_predicted_impact);
		this->combined_score += combined_misguess;

		run_helper.num_actions_limit = -1;

		this->state_iter++;
		if (this->state_iter >= NUM_DATAPOINTS) {
			this->combined_score /= NUM_DATAPOINTS;

			this->branch_weight = (double)this->branch_count / (double)(this->original_count + this->branch_count);

			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 == 0) {
			#else
			if (this->branch_weight > PASS_THROUGH_BRANCH_WEIGHT
					&& this->new_average_score <= this->existing_average_score) {
			#endif
				this->is_pass_through = true;
			} else {
				this->is_pass_through = false;
			}

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->branch_weight > 0.01
					&& this->combined_score <= this->existing_average_score) {
			#endif /* MDEBUG */
				this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

				this->state = ORIENTATION_EXPERIMENT_STATE_VERIFY_EXISTING;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}