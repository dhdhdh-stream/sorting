#include "helpers.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void random_exit_fetch_context_helper(
		ScopeHistory* scope_history,
		int target_index,
		int& curr_index,
		vector<int>& exit_node_context) {
	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index] == NULL) {
				if (curr_index == target_index) {
					exit_node_context.push_back(-1);
					return;
				}
				curr_index++;
			} else {
				if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];

					exit_node_context.push_back(scope_history->node_histories[i_index][h_index]->node->id);

					random_exit_fetch_context_helper(
						scope_node_history->inner_scope_history,
						target_index,
						curr_index,
						exit_node_context);

					if (curr_index < target_index) {
						exit_node_context.pop_back();

						if (!scope_node_history->is_halfway) {
							if (curr_index == target_index) {
								exit_node_context.push_back(scope_history->node_histories[i_index][h_index]->node->id);
								return;
							}
							curr_index++;
						}
					} else {
						return;
					}
				} else {
					if (curr_index == target_index) {
						exit_node_context.push_back(scope_history->node_histories[i_index][h_index]->node->id);
						return;
					}
					curr_index++;
				}
			}
		}
	}
}

void random_exit(vector<int>& starting_scope_context,
				 vector<int>& starting_node_context,
				 int& new_exit_depth,
				 int& new_exit_node_id) {
	Scope* parent_scope = solution->scopes[starting_scope_context[0]];

	vector<int> scope_context{starting_scope_context[0]};
	vector<int> node_context{-1};

	vector<int> starting_node_ids;
	for (int c_index = 0; c_index < (int)starting_scope_context.size(); c_index++) {
		starting_node_ids.push_back(starting_node_context[c_index]);
	}

	int num_nodes = 0;
	ScopeHistory* scope_history = new ScopeHistory(parent_scope);

	// unused
	int inner_exit_depth = -1;
	int inner_exit_node_id = -1;

	parent_scope->random_exit_activate(starting_node_ids,
									   scope_context,
									   node_context,
									   inner_exit_depth,
									   inner_exit_node_id,
									   num_nodes,
									   scope_history);

	if (num_nodes <= 1) {
		new_exit_depth = 0;
		new_exit_node_id = -1;
	} else {
		uniform_int_distribution<int> distribution(0, num_nodes-2);
		int rand_index = 1 + distribution(generator);
		/**
		 * - skip first node
		 */

		vector<int> exit_node_context;
		int curr_index = 0;
		random_exit_fetch_context_helper(
			scope_history,
			rand_index,
			curr_index,
			exit_node_context);

		new_exit_depth = (int)starting_scope_context.size() - (int)exit_node_context.size();
		new_exit_node_id = exit_node_context.back();
	}

	delete scope_history;
}
