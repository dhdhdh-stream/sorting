#include "solution_helpers.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "pass_through_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "transition_network.h"

using namespace std;

void update_helper(ScopeHistory* scope_history,
				   double target_val) {
	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)node;
				noop_node->curr_num_instances++;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (!action_node->is_generic) {
					action_node->curr_num_instances++;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[h_index];
				ScopeNode* scope_node = (ScopeNode*)node;

				update_helper(scope_node_history->scope_history,
							  target_val);

				scope_node->curr_num_instances++;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[h_index];
				BranchNode* branch_node = (BranchNode*)node;
				if (branch_node_history->is_branch) {
					branch_node->branch_curr_num_instances++;
				} else {
					branch_node->original_curr_num_instances++;
				}
			}
			break;
		}
	}
}

void update_helper(double target_val,
				   SolutionWrapper* wrapper) {
	wrapper->solution->curr_score = 0.999*wrapper->solution->curr_score + 0.001*target_val;

	update_helper(wrapper->scope_histories[0],
				  target_val);

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			switch (it->second->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)it->second;
					noop_node->average_instances_per_run = 0.999*noop_node->average_instances_per_run + 0.001*noop_node->curr_num_instances;
					if (noop_node->curr_num_instances > 0) {
						noop_node->average_instances_per_hit = 0.999*noop_node->average_instances_per_hit + 0.001*noop_node->curr_num_instances;

						noop_node->curr_num_instances = 0;
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)it->second;
					if (!action_node->is_generic) {
						action_node->average_instances_per_run = 0.999*action_node->average_instances_per_run + 0.001*action_node->curr_num_instances;
						if (action_node->curr_num_instances > 0) {
							action_node->average_instances_per_hit = 0.999*action_node->average_instances_per_hit + 0.001*action_node->curr_num_instances;

							action_node->curr_num_instances = 0;
						}
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)it->second;
					scope_node->average_instances_per_run = 0.999*scope_node->average_instances_per_run + 0.001*scope_node->curr_num_instances;
					if (scope_node->curr_num_instances > 0) {
						scope_node->average_instances_per_hit = 0.999*scope_node->average_instances_per_hit + 0.001*scope_node->curr_num_instances;

						scope_node->curr_num_instances = 0;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->original_average_instances_per_run = 0.999*branch_node->original_average_instances_per_run + 0.001*branch_node->original_curr_num_instances;
					if (branch_node->original_curr_num_instances > 0) {
						branch_node->original_average_instances_per_hit = 0.999*branch_node->original_average_instances_per_hit + 0.001*branch_node->original_curr_num_instances;

						branch_node->original_curr_num_instances = 0;
					}
					branch_node->branch_average_instances_per_run = 0.999*branch_node->branch_average_instances_per_run + 0.001*branch_node->branch_curr_num_instances;
					if (branch_node->branch_curr_num_instances > 0) {
						branch_node->branch_average_instances_per_hit = 0.999*branch_node->branch_average_instances_per_hit + 0.001*branch_node->branch_curr_num_instances;

						branch_node->branch_curr_num_instances = 0;
					}
				}
				break;
			}
		}
	}
}
