#include "solution.h"

using namespace std;


// TODO: randomly select branches to create a path of nodes
// - then select inputs
// - then run until end of containing scope
// - then continue a further number of nodes
//   - 50% go into scope nodes
// - then select outputs
// - then construct new scopes, with inputs modified based on selected output

void Solution::random_run(Scope* scope,
						  vector<int>& starting_node_ids,
						  vector<int>& scope_context,
						  vector<int>& node_context,
						  int& exit_depth,
						  int& exit_node_id,
						  vector<>) {

}

void Solution::random_starting_node(Scope* scope,
									int& starting_node_id) {
	vector<int> possible_ids;
	for (int n_index = 0; n_index < (int)scope->nodes.size(); n_index++) {
		if (scope[n_index] != NODE_TYPE_EXIT) {
			possible_ids.push_back(n_index);
		}
	}

	int rand_index = rand()%(int)possible_ids.size();
	starting_node_id = possible_ids[rand_index];
}

void Solution::random_halfway_start_fetch_context_helper(
		ScopeHistory* scope_history,
		int target_index,
		int& curr_index,
		vector<int>& scope_context,
		vector<int>& node_context) {
	int scope_id = scope_history->scope->id;

	for (int h_index = 0; h_index < (int)scope_history->node_histories[0].size(); h_index++) {
		if (scope_history->node_histories[0][h_index]->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];

			scope_context.push_back(scope_id);
			node_context.push_back(scope_history->node_histories[0][h_index]->node->id);

			random_halfway_start_fetch_context_helper(
				scope_node_history->inner_scope_history,
				target_index,
				curr_index,
				scope_context,
				node_context);

			if (curr_index < target_index) {
				scope_context.pop_back();
				node_context.pop_back();

				if (!scope_node_history->is_halfway) {
					if (curr_index == target_index) {
						scope_context.push_back(scope_id);
						node_context.push_back(scope_history->node_histories[0][h_index]->node->id);
						return;
					}
					curr_index++;
				}
			} else {
				return;
			}
		} else {
			if (curr_index == target_index) {
				scope_context.push_back(scope_id);
				node_context.push_back(scope_history->node_histories[0][h_index]->node->id);
				return;
			}
			curr_index++;
		}
	}
}

void Solution::random_halfway_start(ScopeNode* starting_scope_node,
									vector<int>& starting_halfway_scope_context,
									vector<int>& starting_halfway_node_context) {
	Scope* scope = solution->scopes[starting_scope_node->inner_scope_id];

	vector<int> scope_context{-1};
	vector<int> node_context{-1};

	vector<int> starting_node_ids{0};

	int num_nodes = 0;
	ScopeHistory* scope_history = new ScopeHistory(scope);

	// unused
	int inner_exit_depth;
	int inner_exit_node_id;

	scope->random_activate(starting_node_ids,
						   scope_context,
						   node_context,
						   inner_exit_depth,
						   inner_exit_node_id,
						   num_nodes,
						   scope_history);

	int rand_index = rand()%num_nodes;

	int curr_index = 0;

	random_halfway_start_fetch_context_helper(
		scope_history,
		rand_index,
		curr_index,
		starting_halfway_scope_context,
		starting_halfway_node_context);


}
