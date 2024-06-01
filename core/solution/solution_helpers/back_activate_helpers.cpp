#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_possible_helper(vector<Scope*>& scope_context,
							vector<AbstractNode*>& node_context,
							vector<vector<Scope*>>& possible_scope_contexts,
							vector<vector<AbstractNode*>>& possible_node_contexts,
							vector<int>& possible_obs_indexes,
							ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->first;

				if (it->second->index == 0
						|| action_node->next_node != NULL) {
					node_context.back() = it->first;

					for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
						possible_scope_contexts.push_back(scope_context);
						possible_node_contexts.push_back(node_context);
						possible_obs_indexes.push_back(o_index);
					}

					node_context.back() = NULL;
				}
				/**
				 * - don't include potential new ending node for info experiments
				 */
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				node_context.back() = it->first;

				uniform_int_distribution<int> distribution(0, 2);
				if (distribution(generator) != 0) {
					gather_possible_helper(scope_context,
										   node_context,
										   possible_scope_contexts,
										   possible_node_contexts,
										   possible_obs_indexes,
										   scope_node_history->scope_history);
				}

				node_context.back() = NULL;
			}

			break;
		case NODE_TYPE_BRANCH:
			{
				node_context.back() = it->first;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(-1);

				node_context.back() = NULL;
			}

			break;
		case NODE_TYPE_INFO_SCOPE:
			{
				node_context.back() = it->first;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(-1);

				node_context.back() = NULL;
			}

			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				node_context.back() = it->first;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(-1);

				node_context.back() = NULL;
			}

			break;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}
