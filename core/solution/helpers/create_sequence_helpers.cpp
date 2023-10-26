#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_stub_node.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"

using namespace std;

const double SET_INPUT_PERCENTAGE = 0.4;
/**
 * - evaluate SET_OUTPUT_PERCENTAGE, but leave a proportion empty
 */
const double SET_OUTPUT_PERCENTAGE = 0.2;

/**
 * - need to select containing_scope that enables full range
 *   - so not just outer scopes, but also scopes reachable
 *     - start from outer, then 50/50 to recurse inwards
 *   - don't random activate -> trim context
 *     - early exit may exit to a layer outside of bounds
 * 
 * - include all nodes as possibilities even if can no longer reach
 *   - might have been a mistaken pass_through anyways
 */
void random_starting_node(Scope* containing_scope,
						  int& starting_node_id) {
	vector<int> possible_ids;
	for (int n_index = 0; n_index < (int)containing_scope->nodes.size(); n_index++) {
		bool should_add = true;
		if (containing_scope->nodes[n_index]->type == NODE_TYPE_EXIT) {
			should_add = false;
		}
		if (should_add) {
			possible_ids.push_back(n_index);
		}
	}

	uniform_int_distribution<int> distribution(0, (int)possible_ids.size()-1);
	int rand_index = distribution(generator);
	starting_node_id = possible_ids[rand_index];
}

void random_halfway_start_fetch_context_helper(
		ScopeHistory* scope_history,
		int target_index,
		int& curr_index,
		vector<int>& starting_halfway_node_context) {
	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];

				starting_halfway_node_context.push_back(scope_history->node_histories[i_index][h_index]->node->id);

				random_halfway_start_fetch_context_helper(
					scope_node_history->inner_scope_history,
					target_index,
					curr_index,
					starting_halfway_node_context);

				if (curr_index <= target_index) {
					starting_halfway_node_context.pop_back();

					if (!scope_node_history->is_halfway) {
						curr_index++;
						if (curr_index > target_index) {
							starting_halfway_node_context.push_back(scope_history->node_histories[i_index][h_index]->node->id);
							return;
						}
					}
				} else {
					return;
				}
			} else {
				curr_index++;
				if (curr_index > target_index) {
					starting_halfway_node_context.push_back(scope_history->node_histories[i_index][h_index]->node->id);
					return;
				}
			}
		}
	}
}

void random_halfway_start(ScopeNode* starting_scope_node,
						  vector<int>& starting_halfway_node_context) {
	while (true) {
		vector<int> scope_context{-1};
		vector<int> node_context{-1};

		vector<int> starting_node_ids = starting_scope_node->starting_node_ids;

		int num_nodes = 0;
		ScopeHistory* scope_history = new ScopeHistory(starting_scope_node->inner_scope);

		// unused
		int inner_exit_depth = -1;
		int inner_exit_node_id = -1;

		starting_scope_node->inner_scope->random_activate(
			starting_node_ids,
			scope_context,
			node_context,
			inner_exit_depth,
			inner_exit_node_id,
			num_nodes,
			scope_history);

		if (num_nodes > 0) {
			uniform_int_distribution<int> distribution(0, num_nodes-1);
			int rand_index = distribution(generator);

			int curr_index = 0;

			random_halfway_start_fetch_context_helper(
				scope_history,
				rand_index,
				curr_index,
				starting_halfway_node_context);

			delete scope_history;

			break;
		} else {
			delete scope_history;
		}
	}
}

Sequence* create_sequence(Problem& problem,
						  vector<ContextLayer>& context,
						  int explore_context_depth,
						  Scope* containing_scope,
						  RunHelper& run_helper) {
	Sequence* new_sequence = new Sequence();

	/**
	 * - allow score state as input
	 */
	vector<int> possible_input_scope_depths;
	vector<bool> possible_input_outer_is_local;
	vector<int> possible_input_outer_indexes;
	{
		for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
				it != context.back().input_state_vals.end(); it++) {
			possible_input_scope_depths.push_back(0);
			possible_input_outer_is_local.push_back(false);
			possible_input_outer_indexes.push_back(it->first);
		}

		for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
				it != context.back().local_state_vals.end(); it++) {
			possible_input_scope_depths.push_back(0);
			possible_input_outer_is_local.push_back(true);
			possible_input_outer_indexes.push_back(it->first);
		}
	}
	for (int c_index = 1; c_index < explore_context_depth; c_index++) {
		Scope* scope = solution->scopes[context[context.size()-1 - c_index].scope_id];
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[context[context.size()-1 - c_index].node_id];

		for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
				it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
			bool passed_down = false;
			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				if (scope_node->input_inner_layers[i_index] == 0
						&& scope_node->input_inner_is_local[i_index] == false
						&& scope_node->input_outer_is_local[i_index] == false
						&& scope_node->input_outer_indexes[i_index] == it->first) {
					passed_down = true;
					break;
				}
			}
			/**
			 * - difficult to catch state that is halfway passed down then back up
			 *   - but OK, as just means copying then using outdated value
			 *   - so only check input_inner_layer == 0
			 */

			if (!passed_down) {
				possible_input_scope_depths.push_back(c_index);
				possible_input_outer_is_local.push_back(false);
				possible_input_outer_indexes.push_back(it->first);
			}
		}

		for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].local_state_vals.begin();
				it != context[context.size()-1 - c_index].local_state_vals.end(); it++) {
			bool passed_down = false;
			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				if (scope_node->input_inner_layers[i_index] == 0
						&& scope_node->input_inner_is_local[i_index] == false
						&& scope_node->input_outer_is_local[i_index] == true
						&& scope_node->input_outer_indexes[i_index] == it->first) {
					passed_down = true;
					break;
				}
			}

			if (!passed_down) {
				possible_input_scope_depths.push_back(c_index);
				possible_input_outer_is_local.push_back(true);
				possible_input_outer_indexes.push_back(it->first);
			}
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

	vector<map<pair<bool,int>, int>> state_mappings(1);
	int new_num_input_states = 0;
	vector<AbstractNode*> new_nodes;
	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->action = Action(ACTION_NOOP);
	new_nodes.push_back(starting_noop_node);

	int curr_node_id;
	int target_num_nodes;

	bool is_halfway_start = false;
	if (containing_scope->nodes[starting_node_id]->type == NODE_TYPE_SCOPE) {
		// TODO: check !original_starting_scope_node->inner_scope->is_loop
		is_halfway_start = true;
	}
	if (is_halfway_start) {
		ScopeNode* original_starting_scope_node = (ScopeNode*)containing_scope->nodes[starting_node_id];

		vector<int> starting_halfway_node_context;
		random_halfway_start(original_starting_scope_node,
							 starting_halfway_node_context);

		ScopeNode* new_starting_scope_node = new ScopeNode();

		new_starting_scope_node->inner_scope = original_starting_scope_node->inner_scope;
		new_starting_scope_node->starting_node_ids = starting_halfway_node_context;

		vector<int> possible_inner_layers;
		vector<bool> possible_inner_is_local;
		vector<int> possible_inner_indexes;
		{
			for (map<int, pair<bool,double>>::iterator it = containing_scope->input_state_weights.begin();
					it != containing_scope->input_state_weights.end(); it++) {
				if (!it->second.first) {
					possible_inner_layers.push_back(-1);
					possible_inner_is_local.push_back(false);
					possible_inner_indexes.push_back(it->first);
				}
			}

			for (map<int, pair<bool,double>>::iterator it = containing_scope->local_state_weights.begin();
					it != containing_scope->local_state_weights.end(); it++) {
				if (!it->second.first) {
					possible_inner_layers.push_back(-1);
					possible_inner_is_local.push_back(true);
					possible_inner_indexes.push_back(it->first);
				}
			}
		}
		Scope* curr_scope = original_starting_scope_node->inner_scope;
		for (int l_index = 0; l_index < (int)starting_halfway_node_context.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[starting_halfway_node_context[l_index]];

			for (map<int, pair<bool,double>>::iterator it = curr_scope->local_state_weights.begin();
					it != curr_scope->local_state_weights.end(); it++) {
				if (!it->second.first) {
					possible_inner_layers.push_back(l_index);
					possible_inner_is_local.push_back(true);
					possible_inner_indexes.push_back(it->first);
				}
			}

			curr_scope = scope_node->inner_scope;
		}
		{
			for (map<int, pair<bool,double>>::iterator it = curr_scope->local_state_weights.begin();
					it != curr_scope->local_state_weights.end(); it++) {
				if (!it->second.first) {
					possible_inner_layers.push_back((int)starting_halfway_node_context.size()-1);
					possible_inner_is_local.push_back(true);
					possible_inner_indexes.push_back(it->first);
				}
			}
		}

		vector<map<int, StateStatus>> halfway_inner_input_state_vals(starting_halfway_node_context.size());
		vector<map<int, StateStatus>> halfway_inner_local_state_vals(starting_halfway_node_context.size());

		int num_inputs_to_consider = (int)(SET_INPUT_PERCENTAGE*(double)possible_inner_layers.size());

		uniform_int_distribution<int> empty_distribution(0, 1);
		uniform_int_distribution<int> input_type_distribution(0, 1);
		uniform_int_distribution<int> init_distribution(0, 1);
		for (int i_index = 0; i_index < num_inputs_to_consider; i_index++) {
			uniform_int_distribution<int> target_distribution(0, (int)possible_inner_layers.size()-1);
			int rand_target = target_distribution(generator);

			if (empty_distribution(generator) == 0) {
				// zero
				double init_val = 0.0;

				if (possible_inner_layers[rand_target] == -1) {
					if (possible_inner_is_local[rand_target]) {
						temp_context.back().local_state_vals[possible_inner_indexes[rand_target]] = StateStatus(init_val);
					} else {
						temp_context.back().input_state_vals[possible_inner_indexes[rand_target]] = StateStatus(init_val);
					}

					int new_state_index;
					state_mappings.back()[{possible_inner_is_local[rand_target], possible_inner_indexes[rand_target]}] = new_num_input_states;
					new_state_index = new_num_input_states;
					new_num_input_states++;

					for (int i_index = 0; i_index < (int)original_starting_scope_node->input_types.size(); i_index++) {
						if (original_starting_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
							if (original_starting_scope_node->input_inner_layers[i_index] == 0
									&& !original_starting_scope_node->input_inner_is_local[i_index]
									&& original_starting_scope_node->input_outer_is_local[i_index] == possible_inner_is_local[rand_target]
									&& original_starting_scope_node->input_outer_indexes[i_index] == possible_inner_indexes[rand_target]) {
								new_starting_scope_node->input_types.push_back(INPUT_TYPE_STATE);
								new_starting_scope_node->input_inner_layers.push_back(0);
								new_starting_scope_node->input_inner_is_local.push_back(original_starting_scope_node->input_inner_is_local[i_index]);
								new_starting_scope_node->input_inner_indexes.push_back(original_starting_scope_node->input_inner_indexes[i_index]);
								new_starting_scope_node->input_outer_is_local.push_back(false);
								new_starting_scope_node->input_outer_indexes.push_back(new_state_index);
								new_starting_scope_node->input_init_vals.push_back(0.0);

								break;
							}
						}
					}

					new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(-1);
					new_sequence->input_outer_is_local.push_back(false);
					new_sequence->input_outer_indexes.push_back(-1);
					new_sequence->input_init_vals.push_back(init_val);
				} else {
					// possible_inner_is_local[rand_target] == true
					halfway_inner_local_state_vals[possible_inner_layers[rand_target]][possible_inner_indexes[rand_target]] = StateStatus(init_val);

					new_starting_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_starting_scope_node->input_inner_layers.push_back(possible_inner_layers[rand_target]);
					new_starting_scope_node->input_inner_is_local.push_back(true);
					new_starting_scope_node->input_inner_indexes.push_back(possible_inner_indexes[rand_target]);
					new_starting_scope_node->input_outer_is_local.push_back(false);
					new_starting_scope_node->input_outer_indexes.push_back(-1);
					new_starting_scope_node->input_init_vals.push_back(init_val);
				}
			} else {
				if (input_type_distribution(generator) == 0 && possible_input_scope_depths.size() > 0) {
					// state
					uniform_int_distribution<int> input_distribution(0, (int)possible_input_scope_depths.size()-1);
					int rand_input = input_distribution(generator);

					map<int, StateStatus>::iterator val_it;
					if (possible_input_outer_is_local[rand_input]) {
						val_it = context[context.size()-1 - possible_input_scope_depths[rand_input]]
							.local_state_vals.find(possible_input_outer_indexes[rand_input]);
					} else {
						val_it = context[context.size()-1 - possible_input_scope_depths[rand_input]]
							.input_state_vals.find(possible_input_outer_indexes[rand_input]);
					}

					int new_state_index;
					if (possible_inner_layers[rand_target] == -1) {
						if (possible_inner_is_local[rand_target]) {
							temp_context.back().local_state_vals[possible_inner_indexes[rand_target]] = val_it->second;
						} else {
							temp_context.back().input_state_vals[possible_inner_indexes[rand_target]] = val_it->second;
						}

						state_mappings.back()[{possible_inner_is_local[rand_target], possible_inner_indexes[rand_target]}] = new_num_input_states;
						new_state_index = new_num_input_states;
						new_num_input_states++;

						for (int i_index = 0; i_index < (int)original_starting_scope_node->input_types.size(); i_index++) {
							if (original_starting_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
								if (original_starting_scope_node->input_inner_layers[i_index] == 0
										&& !original_starting_scope_node->input_inner_is_local[i_index]
										&& original_starting_scope_node->input_outer_is_local[i_index] == possible_inner_is_local[rand_target]
										&& original_starting_scope_node->input_outer_indexes[i_index] == possible_inner_indexes[rand_target]) {
									new_starting_scope_node->input_types.push_back(INPUT_TYPE_STATE);
									new_starting_scope_node->input_inner_layers.push_back(0);
									new_starting_scope_node->input_inner_is_local.push_back(original_starting_scope_node->input_inner_is_local[i_index]);
									new_starting_scope_node->input_inner_indexes.push_back(original_starting_scope_node->input_inner_indexes[i_index]);
									new_starting_scope_node->input_outer_is_local.push_back(false);
									new_starting_scope_node->input_outer_indexes.push_back(new_state_index);
									new_starting_scope_node->input_init_vals.push_back(0.0);

									break;
								}
							}
						}
					} else {
						// possible_inner_is_local[rand_target] == true
						halfway_inner_local_state_vals[possible_inner_layers[rand_target]][possible_inner_indexes[rand_target]] = val_it->second;

						new_state_index = new_num_input_states;
						new_num_input_states++;

						new_starting_scope_node->input_types.push_back(INPUT_TYPE_STATE);
						new_starting_scope_node->input_inner_layers.push_back(possible_inner_layers[rand_target]);
						new_starting_scope_node->input_inner_is_local.push_back(true);
						new_starting_scope_node->input_inner_indexes.push_back(possible_inner_indexes[rand_target]);
						new_starting_scope_node->input_outer_is_local.push_back(false);
						new_starting_scope_node->input_outer_indexes.push_back(new_state_index);
						new_starting_scope_node->input_init_vals.push_back(0.0);
					}

					new_sequence->input_types.push_back(INPUT_TYPE_STATE);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(possible_input_scope_depths[rand_input]);
					new_sequence->input_outer_is_local.push_back(possible_input_outer_is_local[rand_input]);
					new_sequence->input_outer_indexes.push_back(possible_input_outer_indexes[rand_input]);
					new_sequence->input_init_vals.push_back(0.0);

					// can duplicate inner, so don't remove
				} else {
					// constant
					double init_val = 2*init_distribution(generator)-1;

					if (possible_inner_layers[rand_target] == -1) {
						if (possible_inner_is_local[rand_target]) {
							temp_context.back().local_state_vals[possible_inner_indexes[rand_target]] = StateStatus(init_val);
						} else {
							temp_context.back().input_state_vals[possible_inner_indexes[rand_target]] = StateStatus(init_val);
						}

						int new_state_index;
						state_mappings.back()[{possible_inner_is_local[rand_target], possible_inner_indexes[rand_target]}] = new_num_input_states;
						new_state_index = new_num_input_states;
						new_num_input_states++;

						for (int i_index = 0; i_index < (int)original_starting_scope_node->input_types.size(); i_index++) {
							if (original_starting_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
								if (original_starting_scope_node->input_inner_layers[i_index] == 0
										&& !original_starting_scope_node->input_inner_is_local[i_index]
										&& original_starting_scope_node->input_outer_is_local[i_index] == possible_inner_is_local[rand_target]
										&& original_starting_scope_node->input_outer_indexes[i_index] == possible_inner_indexes[rand_target]) {
									new_starting_scope_node->input_types.push_back(INPUT_TYPE_STATE);
									new_starting_scope_node->input_inner_layers.push_back(0);
									new_starting_scope_node->input_inner_is_local.push_back(original_starting_scope_node->input_inner_is_local[i_index]);
									new_starting_scope_node->input_inner_indexes.push_back(original_starting_scope_node->input_inner_indexes[i_index]);
									new_starting_scope_node->input_outer_is_local.push_back(false);
									new_starting_scope_node->input_outer_indexes.push_back(new_state_index);
									new_starting_scope_node->input_init_vals.push_back(0.0);

									break;
								}
							}
						}

						new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
						new_sequence->input_inner_indexes.push_back(new_state_index);
						new_sequence->input_scope_depths.push_back(-1);
						new_sequence->input_outer_is_local.push_back(false);
						new_sequence->input_outer_indexes.push_back(-1);
						new_sequence->input_init_vals.push_back(init_val);
					} else {
						// possible_inner_is_local[rand_target] == true
						halfway_inner_local_state_vals[possible_inner_layers[rand_target]][possible_inner_indexes[rand_target]] = StateStatus(init_val);

						new_starting_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
						new_starting_scope_node->input_inner_layers.push_back(possible_inner_layers[rand_target]);
						new_starting_scope_node->input_inner_is_local.push_back(true);
						new_starting_scope_node->input_inner_indexes.push_back(possible_inner_indexes[rand_target]);
						new_starting_scope_node->input_outer_is_local.push_back(false);
						new_starting_scope_node->input_outer_indexes.push_back(-1);
						new_starting_scope_node->input_init_vals.push_back(init_val);
					}
				}
			}

			possible_inner_layers.erase(possible_inner_layers.begin() + rand_target);
			possible_inner_is_local.erase(possible_inner_is_local.begin() + rand_target);
			possible_inner_indexes.erase(possible_inner_indexes.begin() + rand_target);
		}

		vector<int> starting_node_ids_copy = starting_halfway_node_context;

		original_starting_scope_node->simple_halfway_activate(
			starting_node_ids_copy,
			halfway_inner_input_state_vals,
			halfway_inner_local_state_vals,
			problem,
			temp_context,
			run_helper);

		for (int o_index = 0; o_index < (int)original_starting_scope_node->output_inner_indexes.size(); o_index++) {
			map<pair<bool,int>, int>::iterator new_state_it
				= state_mappings.back().find({original_starting_scope_node->output_outer_is_local[o_index],
					original_starting_scope_node->output_outer_indexes[o_index]});
			/**
			 * - at top layer, only local states in state_mappings initialized
			 */
			if (new_state_it != state_mappings.back().end()) {
				new_starting_scope_node->output_inner_indexes.push_back(original_starting_scope_node->output_inner_indexes[o_index]);
				new_starting_scope_node->output_outer_is_local.push_back(false);
				new_starting_scope_node->output_outer_indexes.push_back(new_state_it->second);
			}
		}

		for (int n_index = 0; n_index < (int)original_starting_scope_node->state_is_local.size(); n_index++) {
			map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back()
				.find({original_starting_scope_node->state_is_local[n_index],
					original_starting_scope_node->state_indexes[n_index]});
			if (new_state_it != state_mappings.back().end()) {
				new_starting_scope_node->state_is_local.push_back(false);
				new_starting_scope_node->state_indexes.push_back(new_state_it->second);
				new_starting_scope_node->state_obs_indexes.push_back(original_starting_scope_node->state_obs_indexes[n_index]);
				new_starting_scope_node->state_defs.push_back(original_starting_scope_node->state_defs[n_index]);
				new_starting_scope_node->state_network_indexes.push_back(original_starting_scope_node->state_network_indexes[n_index]);
			}
		}

		new_nodes.push_back(new_starting_scope_node);

		curr_node_id = original_starting_scope_node->next_node_id;
		geometric_distribution<int> geometric_distribution(0.3);
		target_num_nodes = geometric_distribution(generator);
	} else {
		vector<bool> possible_inner_is_local;
		vector<int> possible_inner_indexes;
		for (map<int, pair<bool,double>>::iterator it = containing_scope->input_state_weights.begin();
				it != containing_scope->input_state_weights.end(); it++) {
			if (!it->second.first) {
				possible_inner_is_local.push_back(false);
				possible_inner_indexes.push_back(it->first);
			}
		}
		for (map<int, pair<bool,double>>::iterator it = containing_scope->local_state_weights.begin();
				it != containing_scope->local_state_weights.end(); it++) {
			if (!it->second.first) {
				possible_inner_is_local.push_back(true);
				possible_inner_indexes.push_back(it->first);
			}
		}

		int num_inputs_to_consider = (int)(SET_INPUT_PERCENTAGE*(double)possible_inner_layers.size());

		uniform_int_distribution<int> empty_distribution(0, 1);
		uniform_int_distribution<int> input_type_distribution(0, 1);
		uniform_int_distribution<int> init_distribution(0, 1);
		for (int i_index = 0; i_index < num_inputs_to_consider; i_index++) {
			uniform_int_distribution<int> target_distribution(0, (int)possible_inner_indexes.size()-1);
			int rand_target = target_distribution(generator);

			if (empty_distribution(generator) == 0) {
				// zero
				double init_val = 0.0;

				if (possible_inner_is_local[rand_target]) {
					temp_context.back().local_state_vals[possible_inner_indexes[rand_target]] = StateStatus(init_val);
				} else {
					temp_context.back().input_state_vals[possible_inner_indexes[rand_target]] = StateStatus(init_val);
				}

				int new_state_index;
				state_mappings.back()[{possible_inner_is_local[rand_target], possible_inner_indexes[rand_target]}] = new_num_input_states;
				new_state_index = new_num_input_states;
				new_num_input_states++;

				new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_sequence->input_inner_indexes.push_back(new_state_index);
				new_sequence->input_scope_depths.push_back(-1);
				new_sequence->input_outer_is_local.push_back(false);
				new_sequence->input_outer_indexes.push_back(-1);
				new_sequence->input_init_vals.push_back(init_val);
			} else {
				if (input_type_distribution(generator) == 0 && possible_input_scope_depths.size() > 0) {
					// state
					uniform_int_distribution<int> input_distribution(0, (int)possible_input_scope_depths.size()-1);
					int rand_input = input_distribution(generator);

					map<int, StateStatus>::iterator val_it;
					if (possible_input_outer_is_local[rand_input]) {
						val_it = context[context.size()-1 - possible_input_scope_depths[rand_input]]
							.local_state_vals.find(possible_input_outer_indexes[rand_input]);
					} else {
						val_it = context[context.size()-1 - possible_input_scope_depths[rand_input]]
							.input_state_vals.find(possible_input_outer_indexes[rand_input]);
					}

					if (possible_inner_is_local[rand_target]) {
						temp_context.back().local_state_vals[possible_inner_indexes[rand_target]] = val_it->second;
					} else {
						temp_context.back().input_state_vals[possible_inner_indexes[rand_target]] = val_it->second;
					}

					int new_state_index;
					state_mappings.back()[{possible_inner_is_local[rand_target], possible_inner_indexes[rand_target]}] = new_num_input_states;
					new_state_index = new_num_input_states;
					new_num_input_states++;

					new_sequence->input_types.push_back(INPUT_TYPE_STATE);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(possible_input_scope_depths[rand_input]);
					new_sequence->input_outer_is_local.push_back(possible_input_outer_is_local[rand_input]);
					new_sequence->input_outer_indexes.push_back(possible_input_outer_indexes[rand_input]);
					new_sequence->input_init_vals.push_back(0.0);

					// can duplicate, so don't remove from possible_inputs
				} else {
					// constant
					double init_val = 2*init_distribution(generator)-1;

					if (possible_inner_is_local[rand_target]) {
						temp_context.back().local_state_vals[possible_inner_indexes[rand_target]] = StateStatus(init_val);
					} else {
						temp_context.back().input_state_vals[possible_inner_indexes[rand_target]] = StateStatus(init_val);
					}

					int new_state_index;
					state_mappings.back()[{possible_inner_is_local[rand_target], possible_inner_indexes[rand_target]}] = new_num_input_states;
					new_state_index = new_num_input_states;
					new_num_input_states++;

					new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(-1);
					new_sequence->input_outer_is_local.push_back(false);
					new_sequence->input_outer_indexes.push_back(-1);
					new_sequence->input_init_vals.push_back(init_val);
				}
			}

			possible_inner_is_local.erase(possible_inner_is_local.begin() + rand_target);
			possible_inner_indexes.erase(possible_inner_indexes.begin() + rand_target);
		}

		curr_node_id = starting_node_id;
		geometric_distribution<int> geometric_distribution(0.3);
		target_num_nodes = 1 + geometric_distribution(generator);
	}

	if (target_num_nodes != 0) {
		vector<int> starting_node_ids{curr_node_id};
		vector<map<int, StateStatus>> starting_input_state_vals;
		vector<map<int, StateStatus>> starting_local_state_vals;
		vector<map<pair<bool,int>, int>> starting_state_mappings;
		int curr_num_nodes = 0;
		containing_scope->create_sequence_activate(starting_node_ids,
												   starting_input_state_vals,
												   starting_local_state_vals,
												   starting_state_mappings,
												   problem,
												   temp_context,
												   target_num_nodes,
												   curr_num_nodes,
												   new_sequence,
												   state_mappings,
												   new_num_input_states,
												   new_nodes,
												   run_helper);
	}

	map<int, StateStatus> possible_inner_outputs;
	for (int c_index = 0; c_index < (int)temp_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = temp_context[c_index].input_state_vals.begin();
				it != temp_context[c_index].input_state_vals.end(); it++) {
			int new_state_index = state_mappings[c_index].find({false, it->first})->second;
			possible_inner_outputs[new_state_index] = it->second;
			// may overwrite earlier definition
		}

		for (map<int, StateStatus>::iterator it = temp_context[c_index].local_state_vals.begin();
				it != temp_context[c_index].local_state_vals.end(); it++) {
			int new_state_index = state_mappings[c_index].find({true, it->first})->second;
			possible_inner_outputs[new_state_index] = it->second;
			// may overwrite earlier definition
		}
	}
	if (possible_inner_outputs.size() > 0) {
		/**
		 * - allow score state as output
		 */
		vector<int> possible_output_scope_depths;
		vector<bool> possible_output_outer_is_local;
		vector<int> possible_output_outer_indexes;
		{
			for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
					it != context.back().input_state_vals.end(); it++) {
				possible_output_scope_depths.push_back(0);
				possible_output_outer_is_local.push_back(false);
				possible_output_outer_indexes.push_back(it->first);
			}

			for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
					it != context.back().local_state_vals.end(); it++) {
				possible_output_scope_depths.push_back(0);
				possible_output_outer_is_local.push_back(true);
				possible_output_outer_indexes.push_back(it->first);
			}
		}
		for (int c_index = 1; c_index < explore_context_depth; c_index++) {
			Scope* scope = solution->scopes[context[context.size()-1 - c_index].scope_id];
			ScopeNode* scope_node = (ScopeNode*)scope->nodes[context[context.size()-1 - c_index].node_id];

			for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
					it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
				bool passed_out = false;
				for (int o_index = 0; o_index < (int)scope_node->output_inner_indexes.size(); o_index++) {
					if (scope_node->output_outer_is_local[o_index] == false
							&& scope_node->output_outer_indexes[o_index] == it->first) {
						passed_out = true;
						break;
					}
				}

				if (!passed_out) {
					possible_output_scope_depths.push_back(c_index);
					possible_output_outer_is_local.push_back(false);
					possible_output_outer_indexes.push_back(it->first);
				}
			}

			for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
				bool passed_out = false;
				for (int o_index = 0; o_index < (int)scope_node->output_inner_indexes.size(); o_index++) {
					if (scope_node->output_outer_is_local[o_index] == true
							&& scope_node->output_outer_indexes[o_index] == s_index) {
						passed_out = true;
						break;
					}
				}

				if (!passed_out) {
					possible_output_scope_depths.push_back(c_index);
					possible_output_outer_is_local.push_back(true);
					possible_output_outer_indexes.push_back(s_index);
				}
			}
		}

		vector<int> possible_inner_output_ids;
		for (map<int, StateStatus>::iterator it = possible_inner_outputs.begin();
				it != possible_inner_outputs.end(); it++) {
			possible_inner_output_ids.push_back(it->first);
		}

		int num_outputs_to_consider = (int)(SET_OUTPUT_PERCENTAGE*(double)possible_output_scope_depths.size());

		uniform_int_distribution<int> output_type_distribution(0, 1);
		uniform_int_distribution<int> reuse_distribution(0, 4);
		for (int o_index = 0; o_index < num_outputs_to_consider; o_index++) {
			uniform_int_distribution<int> target_distribution(0, (int)possible_output_scope_depths.size()-1);
			int rand_target = target_distribution(generator);

			if (output_type_distribution(generator) == 0) {
				vector<int> input_indexes;
				for (int i_index = 0; i_index < (int)new_sequence->input_types.size(); i_index++) {
					if (new_sequence->input_types[i_index] == INPUT_TYPE_STATE) {
						if (new_sequence->input_scope_depths[i_index] == possible_output_scope_depths[rand_target]
								&& new_sequence->input_outer_is_local[i_index] == possible_output_outer_is_local[rand_target]
								&& new_sequence->input_outer_indexes[i_index] == possible_output_outer_indexes[rand_target]) {
							int new_input_index = new_sequence->input_inner_indexes[i_index];
							if (possible_inner_outputs.find(new_input_index) != possible_inner_outputs.end()) {
								input_indexes.push_back(new_input_index);
							}
							// might have been removed by branch
						}
					}
				}

				int input_index;
				if (input_indexes.size() > 0 && reuse_distribution(generator) != 0) {
					uniform_int_distribution<int> inner_distribution(0, (int)input_indexes.size()-1);
					input_index = input_indexes[inner_distribution(generator)];
				} else {
					uniform_int_distribution<int> inner_distribution(0, (int)possible_inner_output_ids.size()-1);
					input_index = possible_inner_output_ids[inner_distribution(generator)];
				}

				if (possible_output_outer_is_local[rand_target]) {
					context[context.size()-1 - possible_output_scope_depths[rand_target]]
						.local_state_vals[possible_output_outer_indexes[rand_target]] = possible_inner_outputs[input_index];
				} else {
					context[context.size()-1 - possible_output_scope_depths[rand_target]]
						.input_state_vals[possible_output_outer_indexes[rand_target]] = possible_inner_outputs[input_index];
				}

				new_sequence->output_inner_indexes.push_back(input_index);
				new_sequence->output_scope_depths.push_back(possible_output_scope_depths[rand_target]);
				new_sequence->output_outer_is_local.push_back(possible_output_outer_is_local[rand_target]);
				new_sequence->output_outer_indexes.push_back(possible_output_outer_indexes[rand_target]);

				// can duplicate inner, so don't remove
			} else {
				// do nothing
			}

			possible_output_scope_depths.erase(possible_output_scope_depths.begin() + rand_target);
			possible_output_outer_is_local.erase(possible_output_outer_is_local.begin() + rand_target);
			possible_output_outer_indexes.erase(possible_output_outer_indexes.begin() + rand_target);
		}
	}

	Scope* new_scope = new Scope();

	// don't set id/increment scope_counter until train

	new_scope->input_state_counter = new_num_input_states;
	for (int s_index = 0; s_index < new_num_input_states; s_index++) {
		new_scope->input_state_weights[s_index] = {false, 0.0};
	}
	new_scope->local_state_counter = 0;

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

			new_scope->child_scopes.push_back(scope_node->inner_scope);
		} else {
			BranchStubNode* branch_stub_node = (BranchStubNode*)new_scope->nodes[n_index];
			branch_stub_node->next_node_id = next_node_id;
		}
	}

	Scope* parent_scope = solution->scopes[context[context.size() - explore_context_depth].scope_id];
	new_scope->average_score = parent_scope->average_score;
	new_scope->score_variance = parent_scope->score_variance;
	new_scope->average_misguess = parent_scope->average_misguess;
	new_scope->misguess_variance = parent_scope->misguess_variance;

	new_sequence->scope = new_scope;

	return new_sequence;
}
