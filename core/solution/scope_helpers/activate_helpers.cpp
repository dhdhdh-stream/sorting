#include "scope.h"

#include <iostream>
#include <stdexcept>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void node_activate_helper(int iter_index,
						  AbstractNode*& curr_node,
						  Problem& problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  ScopeHistory* history) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;
		ActionNodeHistory* node_history = new ActionNodeHistory(node);
		history->node_histories[iter_index].push_back(node_history);
		node->activate(curr_node,
					   problem,
					   context,
					   exit_depth,
					   exit_node,
					   run_helper,
					   node_history);
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;
		ScopeNodeHistory* node_history = new ScopeNodeHistory(node);
		history->node_histories[iter_index].push_back(node_history);
		node->activate(curr_node,
					   problem,
					   context,
					   exit_depth,
					   exit_node,
					   run_helper,
					   node_history);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;

		bool is_branch;
		node->activate(is_branch,
					   context,
					   run_helper);

		if (is_branch) {
			curr_node = node->branch_next_node;
		} else {
			curr_node = node->original_next_node;
		}
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		if (node->exit_depth == 0) {
			curr_node = node->exit_node;
		} else {
			exit_depth = node->exit_depth-1;
			exit_node = node->exit_node;
		}
	}
}

void Scope::activate(Problem& problem,
					 vector<ContextLayer>& context,
					 int& exit_depth,
					 AbstractNode*& exit_node,
					 RunHelper& run_helper,
					 ScopeHistory* history) {
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	run_helper.curr_depth++;

	if (this->is_loop) {
		int iter_index = 0;
		while (true) {
			if (iter_index > this->max_iters+3) {
				run_helper.exceeded_limit = true;
				break;
			}

			double continue_score = this->continue_score_mod;
			double halt_score = this->halt_score_mod;

			for (int s_index = 0; s_index < (int)this->loop_state_is_local.size(); s_index++) {
				if (this->loop_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->loop_state_indexes[s_index]);
					if (it != context.back().local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->loop_continue_weights[s_index] * normalized;
							halt_score += this->loop_halt_weights[s_index] * normalized;
						} else {
							continue_score += this->loop_continue_weights[s_index] * it->second.val;
							halt_score += this->loop_halt_weights[s_index] * it->second.val;
						}
					}
				} else {
					map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->loop_state_indexes[s_index]);
					if (it != context.back().input_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->loop_continue_weights[s_index] * normalized;
							halt_score += this->loop_halt_weights[s_index] * normalized;
						} else {
							continue_score += this->loop_continue_weights[s_index] * it->second.val;
							halt_score += this->loop_halt_weights[s_index] * it->second.val;
						}
					}
				}
			}

			if (halt_score > continue_score) {
				/**
				 * - update even if explore
				 *   - cannot result in worst performance as would previously be -1.0 anyways
				 */
				if (iter_index > this->max_iters) {
					this->max_iters = iter_index;
				}
				break;
			} else {
				history->node_histories.push_back(vector<AbstractNodeHistory*>());

				AbstractNode* curr_node = this->starting_node;
				while (true) {
					if (exit_depth != -1
							|| curr_node == NULL
							|| run_helper.exceeded_limit) {
						break;
					}

					node_activate_helper(iter_index,
										 curr_node,
										 problem,
										 context,
										 exit_depth,
										 exit_node,
										 run_helper,
										 history);
				}

				if (exit_depth != -1
						|| run_helper.exceeded_limit) {
					break;
				} else {
					iter_index++;
					// continue
				}
			}
		}
	} else {
		history->node_histories.push_back(vector<AbstractNodeHistory*>());

		AbstractNode* curr_node = this->starting_node;
		while (true) {
			if (exit_depth != -1
					|| curr_node == NULL
					|| run_helper.exceeded_limit) {
				break;
			}

			node_activate_helper(0,
								 curr_node,
								 problem,
								 context,
								 exit_depth,
								 exit_node,
								 run_helper,
								 history);
		}
	}

	if (history->inner_pass_through_experiment != NULL) {
		history->inner_pass_through_experiment->parent_scope_end_activate(
			context,
			run_helper,
			history);
	}
	/**
	 * - triggers once even if multiple due to loops
	 *   - may lead to misguesses not aligning (even more), but hopefully won't be big impact
	 */

	run_helper.curr_depth--;
}
