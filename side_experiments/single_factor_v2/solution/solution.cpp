#include "solution.h"

using namespace std;



/**
 * - need to select containing_scope that enables full range
 *   - so not just outer scopes, but also scopes reachable
 *     - start from outer, then 50/50 to recurse inwards
 *   - don't random activate -> trim context
 *     - early exit may exit to a layer outside of bounds
 */
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
	Sequence* new_sequence = new Sequence();

	/**
	 * - only use established state for input
	 *   - leave score state for branch decision
	 * 
	 * - negative indexing
	 * 
	 * - don't automatically include global (i.e., context[0]) for now
	 */
	vector<int> possible_input_scope_depths;
	vector<int> possible_input_outer_ids;
	for (int c_index = 0; c_index < explore_context_depth; c_index++) {
		for (map<int, double>::iterator it = context[context.size()-1 - c_index].state_vals.begin();
				it != context[context.size()-1 - c_index].state_vals.end(); it++) {
			possible_input_scope_depths.push_back(c_index);
			possible_input_outer_ids.push_back(it->first);
		}
	}

	int starting_node_id;
	random_starting_node(containing_scope,
						 starting_node_id);

	vector<ContextLayer> temp_context;
	temp_context.push_back(ContextLayer());

	temp_context.back().scope_id = -1;
	temp_context.back().node_id = -1;

	// temp_context.back().state_vals initialized to empty

	vector<map<int, int>> state_mappings(1);
	int new_num_states = 0;
	vector<AbstractNode*> new_nodes;
	
	int curr_node_id;
	int target_num_nodes;

	bool is_halfway_start = false;
	if (containing_scope->nodes[starting_node_id]->type == NODE_TYPE_SCOPE) {
		ScopeNode* original_starting_scope_node = (ScopeNode*)containing_scope->nodes[starting_node_id];
		Scope* inner_scope = solution->scopes[original_starting_scope_node->inner_scope_id];
		if (!inner_scope->is_loop) {
			is_halfway_start = true;
		}
	}
	if (is_halfway_start) {
		ScopeNode* original_starting_scope_node = (ScopeNode*)containing_scope->nodes[starting_node_id];

		vector<int> starting_halfway_node_context;
		random_halfway_start(original_starting_scope_node,
							 starting_halfway_node_context);

		ScopeNode* new_starting_scope_node = new ScopeNode();

		new_starting_scope_node->inner_scope_id = original_starting_scope_node->inner_scope_id;
		new_starting_scope_node->starting_node_ids = starting_halfway_node_context;

		vector<int> possible_inner_layers;
		vector<int> possible_inner_ids;

		vector<map<int, int>> output_mappings(starting_halfway_node_context.size());
		/**
		 * - for matching halfway input with outer
		 */

		/**
		 * - don't need to worry about inputs
		 *   - intuitively, halfway start bypasses the input set
		 * 
		 * - do exclude outputs as any set values may be overwritten
		 */
		uniform_int_distribution<int> overwrite_distribution(0, 1);
		{
			vector<bool> possible_ids(containing_scope->num_states, true);
			for (int o_index = 0; o_index < (int)original_starting_scope_node->output_outer_ids.size(); o_index++) {
				possible_ids[original_starting_scope_node->output_outer_ids[o_index]] = false;

				map<int, int>::iterator inner_it = output_mappings[0].find(original_starting_scope_node->output_inner_ids[o_index]);
				if (inner_it != output_mappings[0].end()) {
					if (overwrite_distribution(generator) == 0) {
						output_mappings[0][original_starting_scope_node->output_inner_ids[o_index]] = original_starting_scope_node->output_outer_ids[o_index];
					}
					/**
					 * - if overwritten is selected as input, will fail to match input and output
					 *   - but OK as just means copy by val
					 */
				} else {
					output_mappings[0][original_starting_scope_node->output_inner_ids[o_index]] = original_starting_scope_node->output_outer_ids[o_index];
				}
			}

			for (int s_index = 0; s_index < containing_scope->num_states; s_index++) {
				if (possible_ids[s_index]) {
					possible_inner_layers.push_back(-1);
					possible_inner_ids.push_back(s_index);
				}
			}
		}
		Scope* curr_scope = solution->scopes[original_starting_scope_node->inner_scope_id];
		for (int l_index = 0; l_index < (int)starting_halfway_node_context.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[starting_halfway_node_context[l_index]];

			vector<bool> possible_ids(curr_scope->num_states, true);
			for (int o_index = 0; o_index < (int)scope_node->output_outer_ids.size(); o_index++) {
				possible_ids[scope_node->output_outer_ids[o_index]] = false;

				map<int, int>::iterator outer_it = output_mappings[l_index].find(scope_node->output_outer_ids[o_index]);
				if (outer_it != output_mappings[l_index].end()) {
					map<int, int>::iterator inner_it = output_mappings[l_index+1].find(scope_node->output_inner_ids[o_index]);
					if (inner_it != output_mappings[l_index+1].end()) {
						if (overwrite_distribution(generator) == 0) {
							output_mappings[l_index+1][scope_node->output_inner_ids[o_index]] = outer_it->second;
						}
					} else {
						output_mappings[l_index+1][scope_node->output_inner_ids[o_index]] = outer_it->second;
					}
				}
			}

			for (s_index = 0; s_index < curr_scope->num_states; s_index++) {
				if (possible_ids[s_index]) {
					possible_inner_layers.push_back(l_index);
					possible_inner_ids.push_back(s_index);
				}
			}

			curr_scope = solution->scopes[scope_node->inner_scope_id];
		}
		{
			for (int s_index = 0; s_index < curr_scope->num_states; s_index++) {
				possible_inner_layers.push_back(starting_halfway_node_context.size()-1);
				possible_inner_ids.push_back(s_index);
			}
		}

		vector<map<int, double>> halfway_inner_state_vals(starting_halfway_node_context.size());

		int num_inputs_to_consider = min(20, (int)possible_inner_layers.size());

		uniform_int_distribution<int> input_type_distribution(0, 2);
		uniform_int_distribution<int> init_distribution(0, 1);
		for (int i_index = 0; i_index < num_inputs_to_consider; i_index++) {
			uniform_int_distribution<int> target_distribution(0, possible_inner_layers.size()-1);
			int rand_target = target_distribution(generator);

			int type = input_type_distribution(generator);
			if (type == 0 && possible_input_scope_depths.size() > 0) {
				// state
				uniform_int_distribution<int> input_distribution(0, possible_input_scope_depths.size()-1);
				int rand_input = input_distribution(generator);

				map<int, double> val_it = context[context.size()-1 - possible_input_scope_depths[rand_input]]
					.state_vals.find(possible_input_outer_ids[rand_input]);
				// it != context[context.size()-1 - possible_input_scope_depths[rand_input]].state_vals.end()

				int new_state_id;
				if (possible_inner_layers[rand_target] == -1) {
					temp_context.back().state_vals[possible_inner_ids[rand_target]] = val_it->second;

					state_mappings.back()[possible_inner_ids[rand_target]] = new_num_states;
					new_state_id = new_num_states;
					new_num_states++;
				} else {
					halfway_inner_state_vals[possible_inner_layers[rand_target]][possible_inner_ids[rand_target]] = val;

					map<int, int>::iterator it = output_mappings[possible_inner_layers[rand_target]].find(possible_inner_ids[rand_target]);
					if (it != output_mappings[possible_inner_layers[rand_target]].end()) {
						state_mappings.back()[it->second] = new_num_states;
					}
					new_state_id = new_num_states;
					new_num_states++;

					new_starting_scope_node->input_types.push_back(INPUT_TYPE_STATE);
					new_starting_scope_node->input_inner_layers.push_back(possible_inner_layers[rand_target]);
					new_starting_scope_node->input_inner_ids.push_back(possible_inner_ids[rand_target]);
					new_starting_scope_node->input_outer_ids.push_back(new_state_id);
					new_starting_scope_node->input_init_vals.push_back(0.0);
				}

				new_sequence->input_types.push_back(INPUT_TYPE_STATE);
				new_sequence->input_inner_ids.push_back(new_state_id);
				new_sequence->input_scope_depths.push_back(possible_input_scope_depths[rand_input]);
				new_sequence->input_outer_ids.push_back(possible_input_outer_ids[rand_input]);
				new_sequence->input_init_vals.push_back(0.0);

				possible_input_scope_depths.erase(possible_input_scope_depths.begin() + rand_input);
				possible_input_outer_ids.erase(possible_input_outer_ids.begin() + rand_input);
			} else if (type == 1) {
				// constant
				double init_val = 2*init_distribution(generator)-1;

				if (possible_inner_layers[rand_target] == -1) {
					temp_context.back().state_vals[possible_inner_ids[rand_target]] = init_val;

					int new_state_id;
					state_mappings.back()[possible_inner_ids[rand_target]] = new_num_states;
					new_state_id = new_num_states;
					new_num_states++;

					new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_sequence->input_inner_ids.push_back(new_state_id);
					new_sequence->input_scope_depths.push_back(-1);
					new_sequence->input_outer_ids.push_back(-1);
					new_sequence->input_init_vals.push_back(init_val);
				} else {
					halfway_inner_state_vals[possible_inner_layers[rand_target]][possible_inner_ids[rand_target]] = init_val;

					new_starting_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_starting_scope_node->input_inner_layers.push_back(possible_inner_layers[rand_target]);
					new_starting_scope_node->input_inner_ids.push_back(possible_inner_ids[rand_target]);
					new_starting_scope_node->input_outer_ids.push_back(-1);
					new_starting_scope_node->input_init_vals.push_back(init_val);
				}
			} else {
				// none
			}

			possible_inner_layers.erase(possible_inner_layers.begin() + rand_target);
			possible_inner_ids.erase(possible_inner_ids.begin() + rand_target);
		}

		vector<int> starting_node_ids_copy = starting_halfway_node_context;

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(original_starting_scope_node);
		original_starting_scope_node->activate(starting_node_ids_copy,
											   halfway_inner_state_vals,
											   flat_vals,
											   temp_context,
											   inner_exit_depth,
											   inner_exit_node_id,
											   run_helper,
											   node_history);
		delete node_history;

		for (int o_index = 0; o_index < (int)original_starting_scope_node->output_inner_ids.size(); o_index++) {
			map<int, int>::iterator it = state_mappings.back().find(original_starting_scope_node->output_outer_ids[o_index]);
			if (it == state_mappings.back().end()) {
				it = state_mappings.back().insert({original_starting_scope_node->output_outer_ids[o_index], new_num_states}).first;
				new_num_states++;
			}
			new_starting_scope_node->output_inner_ids.push_back(original_starting_scope_node->output_inner_ids[o_index]);
			new_starting_scope_node->output_outer_ids.push_back(it->second);
		}

		new_nodes.push_back(new_starting_scope_node);

		curr_node_id = original_starting_scope_node->next_node_id;
		geometric_distribution<int> geometric_distribution(0.3);
		target_num_nodes = geometric_distribution(generator);
	} else {
		vector<int> possible_inner_ids;
		for (int s_index = 0; s_index < containing_scope->num_states; s_index++) {
			possible_inner_ids.push_back(s_index);
		}

		int num_inputs_to_consider = min(20, (int)possible_inner_ids.size());

		uniform_int_distribution<int> input_type_distribution(0, 2);
		uniform_int_distribution<int> init_distribution(0, 1);
		for (int i_index = 0; i_index < num_inputs_to_consider; i_index++) {
			uniform_int_distribution<int> target_distribution(0, possible_inner_ids.size()-1);
			int rand_target = target_distribution(generator);

			int type = input_type_distribution(generator);
			if (type == 0 && possible_input_scope_depths.size() > 0) {
				// state
				uniform_int_distribution<int> input_distribution(0, possible_input_scope_depths.size()-1);
				int rand_input = input_distribution(generator);

				map<int, double> val_it = context[context.size()-1 - possible_input_scope_depths[rand_input]]
					.state_vals.find(possible_input_outer_ids[rand_input]);
				// it != context[context.size()-1 - possible_input_scope_depths[rand_input]].state_vals.end()

				temp_context.back().state_vals[possible_inner_ids[rand_target]] = val_it->second;

				int new_state_id;
				state_mappings.back()[possible_inner_ids[rand_target]] = new_num_states;
				new_state_id = new_num_states;
				new_num_states++;

				new_sequence->input_types.push_back(INPUT_TYPE_STATE);
				new_sequence->input_inner_ids.push_back(new_state_id);
				new_sequence->input_scope_depths.push_back(possible_input_scope_depths[rand_input]);
				new_sequence->input_outer_ids.push_back(possible_input_outer_ids[rand_input]);
				new_sequence->input_init_vals.push_back(0.0);

				possible_input_scope_depths.erase(possible_input_scope_depths.begin() + rand_input);
				possible_input_outer_ids.erase(possible_input_outer_ids.begin() + rand_input);
			} else if (type == 1) {
				// constant
				double init_val = 2*init_distribution(generator)-1;

				temp_context.back().state_vals[possible_inner_ids[rand_target]] = init_val;

				int new_state_id;
				state_mappings.back()[possible_inner_ids[rand_target]] = new_num_states;
				new_state_id = new_num_states;
				new_num_states++;

				new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_sequence->input_inner_ids.push_back(new_state_id);
				new_sequence->input_scope_depths.push_back(-1);
				new_sequence->input_outer_ids.push_back(-1);
				new_sequence->input_init_vals.push_back(init_val);
			} else {
				// none
			}

			possible_inner_ids.erase(possible_inner_ids.begin() + rand_target);
		}

		curr_node_id = starting_node_id;
		geometric_distribution<int> geometric_distribution(0.3);
		target_num_nodes = 1 + geometric_distribution(generator);
	}

	if (target_num_nodes != 0) {
		vector<int> starting_node_ids{curr_node_id};
		vector<map<int, double>> starting_state_vals;
		vector<map<int, int>> starting_state_mappings;
		int curr_num_nodes = 0;
		containing_scope->create_sequence_activate(starting_node_ids,
												   starting_state_vals,
												   starting_state_mappings,
												   flat_vals,
												   context,
												   target_num_nodes,
												   curr_num_nodes,
												   new_sequence,
												   state_mappings,
												   new_num_states,
												   new_nodes,
												   run_helper);
	}

	vector<int> possible_output_scope_depths;
	vector<int> possible_output_outer_ids;
	for (int c_index = 0; c_index < explore_context_depth; c_index++) {
		for (map<int, double>::iterator it = context[context.size()-1 - c_index].state_vals.begin();
				it != context[context.size()-1 - c_index].state_vals.end(); it++) {
			possible_output_scope_depths.push_back(c_index);
			possible_output_outer_ids.push_back(it->first);
		}
	}

	map<int, double> possible_inner_outputs;
	for (int c_index = 0; c_index < temp_context.size(); c_index++) {
		for (map<int, double>::iterator it = temp_context[c_index].state_vals.begin();
				it != temp_context[c_index].state_vals.end(); it++) {
			int new_state_id = state_mappings[c_index].find(it->first)->second;
			possible_inner_outputs[new_state_id] = it->second;
			// may overwrite earlier definition
		}
	}
	vector<int> possible_inner_output_ids;
	for (map<int, double>::iterator it = possible_inner_outputs.begin();
			it != possible_inner_outputs.end(); it++) {
		possible_inner_output_ids.push_back(it->first);
	}

	int num_outputs_to_consider = min(20, possible_output_scope_depths.size());

	uniform_int_distribution<int> output_type_distribution(0, 1);
	for (int o_index = 0; o_index < num_outputs_to_consider; o_index++) {
		uniform_int_distribution<int> target_distribution(0, possible_output_scope_depths.size()-1);
		int rand_target = target_distribution(generator);

		int input_id = -1;
		for (int i_index = 0; i_index < (int)new_sequence->input_types.size(); i_index++) {
			if (new_sequence->input_types[i_index] == INPUT_TYPE_STATE) {
				if (new_sequence->input_scope_depths[i_index] == possible_output_scope_depths[rand_target]
						&& new_sequence->input_outer_ids[i_index] == possible_output_outer_ids[rand_target]) {
					input_id = new_sequence->input_inner_ids[i_index];
					break;
				}
			}
		}

		if (input_id != -1) {
			if (output_type_distribution(generator) == 0) {
				context[context.size()-1 - possible_output_scope_depths[rand_target]]
					.state_vals[possible_output_outer_ids[rand_target]] = possible_inner_outputs[input_id];

				new_sequence->output_inner_ids.push_back(input_id);
				new_sequence->output_scope_depths.push_back(possible_output_scope_depths[rand_target]);
				new_sequence->output_outer_ids.push_back(possible_output_outer_ids[rand_target]);
			} else {
				// do nothing
			}
		} else {
			if (output_type_distribution(generator) == 0) {
				uniform_int_distribution<int> inner_distribution(0, possible_inner_output_ids.size()-1);
				int rand_inner = inner_distribution(generator);

				context[context.size()-1 - possible_output_scope_depths[rand_target]]
					.state_vals[possible_output_outer_ids[rand_target]] = possible_inner_outputs[possible_inner_output_ids[rand_inner]];

				new_sequence->output_inner_ids.push_back(possible_inner_output_ids[rand_inner]);
				new_sequence->output_scope_depths.push_back(possible_output_scope_depths[rand_target]);
				new_sequence->output_outer_ids.push_back(possible_output_outer_ids[rand_target]);

				// can duplicate, so don't remove from possible_inner_output_ids
			} else {
				// do nothing
			}
		}

		possible_output_scope_depths.erase(possible_output_scope_depths.begin() + rand_target);
		possible_output_outer_ids.erase(possible_output_outer_ids.begin() + rand_target);
	}

	Scope* new_scope = new Scope();
	new_scope->num_states = new_num_states;
	new_scope->nodes = new_nodes;
	for (int n_index = 0; n_index < (int)new_scope->nodes.size(); n_index++) {
		new_scope->nodes[n_index]->id = n_index;

		int next_node_id;
		if (n_index == (int)new_scope->nodes.size()-1) {
			next_node_id = -1;
		} else {
			next_node_id = n_index+1;
		}

		if (new_scope->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)new_scope->nodes[n_index];
			action_node->next_node_id = next_node_id;
		} else if (new_scope->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)new_scope->nodes[n_index];
			scope_node->next_node_id = next_node_id;
		} else {
			BranchStubNode* branch_stub_node = (BranchStubNode*)new_scope->nodes[n_index];
			branch_stub_node->next_node_id = next_node_id;
		}
	}
}
