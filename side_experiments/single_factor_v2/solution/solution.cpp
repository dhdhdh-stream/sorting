#include "solution.h"

using namespace std;


// TODO: randomly select branches to create a path of nodes
// - then select inputs
// - then run until end of containing scope
// - then continue a further number of nodes
//   - 50% go into scope nodes
// - then select outputs
// - then construct new scopes, with inputs modified based on selected output

void Solution::random_starting_node(Scope* containing_scope,
									int& starting_node_id) {
	vector<int> possible_ids;
	for (int n_index = 0; n_index < (int)containing_scope->nodes.size(); n_index++) {
		if (containing_scope[n_index] != NODE_TYPE_EXIT) {
			possible_ids.push_back(n_index);
		}
	}

	uniform_int_distribution<int> distribution(0, possible_ids.size()-1);
	int rand_index = distribution(generator);
	starting_node_id = possible_ids[rand_index];
}

void Solution::random_halfway_start_fetch_context_helper(
		ScopeHistory* scope_history,
		int target_index,
		int& curr_index,
		vector<int>& starting_halfway_node_context) {
	for (int h_index = 0; h_index < (int)scope_history->node_histories[0].size(); h_index++) {
		if (scope_history->node_histories[0][h_index]->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[0][h_index];

			starting_halfway_node_context.push_back(scope_history->node_histories[0][h_index]->node->id);

			random_halfway_start_fetch_context_helper(
				scope_node_history->inner_scope_history,
				target_index,
				curr_index,
				starting_halfway_node_context);

			if (curr_index < target_index) {
				starting_halfway_node_context.pop_back();

				if (!scope_node_history->is_halfway) {
					if (curr_index == target_index) {
						starting_halfway_node_context.push_back(scope_history->node_histories[0][h_index]->node->id);
						return;
					}
					curr_index++;
				}
			} else {
				return;
			}
		} else {
			if (curr_index == target_index) {
				starting_halfway_node_context.push_back(scope_history->node_histories[0][h_index]->node->id);
				return;
			}
			curr_index++;
		}
	}
}

void Solution::random_halfway_start(ScopeNode* starting_scope_node,
									vector<int>& starting_halfway_node_context) {
	Scope* scope = solution->scopes[starting_scope_node->inner_scope_id];

	vector<int> scope_context{-1};
	vector<int> node_context{-1};

	vector<int> starting_node_ids = starting_scope_node->starting_node_ids;

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

	uniform_int_distribution<int> distribution(0, num_nodes-1);
	int rand_index = distribution(generator);

	int curr_index = 0;

	random_halfway_start_fetch_context_helper(
		scope_history,
		rand_index,
		curr_index,
		starting_halfway_node_context);
}

ScopeNode* Solution::construct_sequence(vector<double>& flat_vals,
										vector<ContextLayer>& context,
										int explore_context_depth,
										Scope* containing_scope,
										RunHelper& run_helper) {
	ScopeNode* new_outer_scope_node = new ScopeNode();
	Scope* new_scope = new Scope();

	/**
	 * - only use established state for input
	 *   - leave score state for branch decision
	 * 
	 * - negative indexing
	 * 
	 * - don't automatically include global (i.e., context[0]) for now
	 */
	vector<int> possible_scope_depths;
	vector<int> possible_input_ids;
	uniform_int_distribution<int> include_distribution(0, 4);
	for (int c_index = 0; c_index < explore_context_depth; c_index++) {
		for (map<int, double>::iterator it = context[context.size()-1 - c_index].state_vals.begin();
				it != context[context.size()-1 - c_index].state_vals.end(); it++) {
			if (include_distribution(generator) == 0) {
				// 20%
				possible_scope_depths.push_back(c_index);
				possible_input_ids.push_back(it->first);
			}
		}
	}

	vector<int> possible_target_layers;
	vector<int> possible_target_ids;

	for (int s_index = 0; s_index < containing_scope->num_states; s_index++) {
		possible_target_layers.push_back(-1);
		possible_target_ids.push_back(s_index);
	}

	int starting_node_id;
	random_starting_node(containing_scope,
						 starting_node_id);

	ScopeNode* new_starting_scope_node;
	if (containing_scope->nodes[starting_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* original_starting_scope_node = (ScopeNode*)containing_scope->nodes[starting_node_id];

		vector<int> starting_halfway_node_context;
		random_halfway_start(original_starting_scope_node,
							 starting_halfway_node_context);

		new_starting_scope_node = new ScopeNode();
		new_starting_scope_node->inner_scope_id = original_starting_scope_node->inner_scope_id;
		new_starting_scope_node->starting_node_ids = starting_halfway_node_context;

		vector<vector<int>> passed_down_states(starting_halfway_node_context.size()-1);

		Scope* curr_scope = solution->scopes[new_starting_scope_node->inner_scope_id];
		// no passed down in first layer as rebuilding
		for (int s_index = 0; s_index < curr_scope->num_states; s_index++) {
			possible_target_layers.push_back(0);
			possible_target_ids.push_back(s_index);
		}
		for (int l_index = 0; l_index < (int)starting_halfway_node_context.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[starting_halfway_node_context[l_index]];
			curr_scope = solution->scopes[scope_node->inner_scope_id];

			int furthest_matching_layer = 0;
			for (int il_index = 0; il_index < scope_node->starting_node_ids.size(); il_index++) {
				if (il_index >= (int)starting_halfway_node_context.size() - l_index
						|| starting_halfway_node_context[l_index+1 + il_index] != scope_node->starting_node_ids[il_index]) {
					break;
				} else {
					furthest_matching_layer++;
				}
			}
			for (int i_index = 0; i_index < (int)scope_node->input_ids.size(); i_index++) {
				if (scope_node->input_target_layers[i_index] <= furthest_matching_layer) {
					passed_down_states[l_index+1 + scope_node->input_target_layers[i_index]].push_back(
						scope_node->input_target_ids[i_index]);
				}
			}

			vector<bool> states_to_include(curr_scope->num_states, true);
			for (int p_index = 0; p_index < passed_down_states[l_index].size(); p_index++) {
				states_to_include[passed_down_states[l_index+1][p_index]] = false;
			}
			for (int s_index = 0; s_index < curr_scope->num_states; s_index++) {
				if (states_to_include[s_index]) {
					possible_target_layers.push_back(l_index+1);
					possible_target_ids.push_back(s_index);
				}
			}
		}
	}

	// HERE
	uniform_int_distribution<int> type_distribution(0, 4);
	uniform_int_distribution<int> reverse_distribution(0, 1);
	uniform_int_distribution<int> init_distribution(0, 1);
	while (possible_target_layers.size() > 0) {
		uniform_int_distribution<int> target_distribution(0, possible_target_layers.size()-1);
		int rand_target = target_distribution(generator);

		int type = type_distribution(generator);
		if (type == 0 && possible_scope_depths.size() > 0) {
			// 20%
			if (possible_target_layers[rand_target] == -1) {
				new_sequence->outer_input_types.push_back(INPUT_TYPE_STATE);
				new_sequence->outer_input_target_ids.push_back(possible_target_ids[rand_target]);
				new_sequence->outer_input_scope_depths.push_back(possible_scope_depths.back());
				new_sequence->outer_input_input_ids.push_back(possible_input_ids.back());
				new_sequence->outer_input_reverse_sign.push_back(reverse_distribution(generator));
				new_sequence->outer_input_init_vals.push_back(0.0);
			} else {
				new_sequence->halfway_input_types.push_back(INPUT_TYPE_STATE);
				new_sequence->halfway_input_target_layers.push_back(possible_target_layers[rand_target]);
				new_sequence->halfway_input_target_ids.push_back(possible_target_ids[rand_target]);
				new_sequence->halfway_input_scope_depths.push_back(possible_scope_depths.back());
				new_sequence->halfway_input_input_ids.push_back(possible_input_ids.back());
				new_sequence->halfway_input_reverse_sign.push_back(reverse_distribution(generator));
				new_sequence->halfway_input_init_vals.push_back(0.0);
			}

			possible_scope_depths.pop_back();
			possible_input_ids.pop_back();
		} else if (type == 1) {
			// 20%
			if (possible_target_layers[rand_target] == -1) {
				new_sequence->outer_input_types.push_back(INPUT_TYPE_CONSTANT);
				new_sequence->outer_input_target_ids.push_back(possible_target_ids[rand_target]);
				new_sequence->outer_input_scope_depths.push_back(-1);
				new_sequence->outer_input_input_ids.push_back(-1);
				new_sequence->outer_input_reverse_sign.push_back(false);
				new_sequence->outer_input_init_vals.push_back(2*init_distribution(generator)-1);
			} else {
				new_sequence->halfway_input_types.push_back(INPUT_TYPE_STATE);
				new_sequence->halfway_input_target_layers.push_back(possible_target_layers[rand_target]);
				new_sequence->halfway_input_target_ids.push_back(possible_target_ids[rand_target]);
				new_sequence->halfway_input_scope_depths.push_back(-1);
				new_sequence->halfway_input_input_ids.push_back(-1);
				new_sequence->halfway_input_reverse_sign.push_back(false);
				new_sequence->halfway_input_init_vals.push_back(2*init_distribution(generator)-1);
			}
		} else {
			// 60%
			// do nothing (i.e., leave at 0.0)
		}

		possible_target_layers.erase(possible_target_layers.begin() + rand_target);
		possible_target_indexes.erase(possible_target_indexes.begin() + rand_target);
	}

	// TODO: activate new scope node, then run create_sequence


	// TODO: construct new scope/nodes on way down
	// - also makes it easy to create output

	// TODO: assign new node ids, parents, next node ids, etc.

}
