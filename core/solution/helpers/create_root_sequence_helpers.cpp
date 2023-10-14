#include "helpers.h"

#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"

using namespace std;

void create_root_random_halfway_start_fetch_context_helper(
		ScopeHistory* scope_history,
		int target_index,
		int& curr_index,
		vector<int>& starting_halfway_node_context) {
	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];

				starting_halfway_node_context.push_back(scope_history->node_histories[i_index][h_index]->node->id);

				create_root_random_halfway_start_fetch_context_helper(
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

void create_root_random_halfway_start(vector<int>& starting_halfway_node_context) {
	Scope* root = solution->scopes[0];

	while (true) {
		vector<int> scope_context{-1};
		vector<int> node_context{-1};

		vector<int> starting_node_ids{0};

		int num_nodes = 0;
		ScopeHistory* scope_history = new ScopeHistory(root);

		// unused
		int inner_exit_depth = -1;
		int inner_exit_node_id = -1;

		root->random_activate(
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

			create_root_random_halfway_start_fetch_context_helper(
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

Sequence* create_root_sequence(Problem& problem,
							   vector<ContextLayer>& context,
							   int explore_context_depth,
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
	vector<bool> possible_input_outer_is_local;
	vector<int> possible_input_outer_indexes;
	// skip context.back()
	{
		for (map<int, StateStatus>::iterator it = context[context.size()-1 - 1].input_state_vals.begin();
				it != context[context.size()-1 - 1].input_state_vals.end(); it++) {
			possible_input_scope_depths.push_back(1);
			possible_input_outer_is_local.push_back(false);
			possible_input_outer_indexes.push_back(it->first);
		}

		for (map<int, StateStatus>::iterator it = context[context.size()-1 - 1].local_state_vals.begin();
				it != context[context.size()-1 - 1].local_state_vals.end(); it++) {
			possible_input_scope_depths.push_back(1);
			possible_input_outer_is_local.push_back(true);
			possible_input_outer_indexes.push_back(it->first);
		}
	}
	for (int c_index = 2; c_index < explore_context_depth; c_index++) {
		Scope* scope = solution->scopes[context[context.size()-1 - c_index].scope_id];
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[context[context.size()-1 - c_index].node_id];

		for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
				it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
			bool passed_down = false;
			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				if (scope_node->input_inner_layers[i_index] == 0
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

	Scope* root = solution->scopes[0];

	int new_num_input_states = 0;

	vector<int> starting_halfway_node_context;
	create_root_random_halfway_start(starting_halfway_node_context);

	ScopeNode* new_starting_scope_node = new ScopeNode();

	new_starting_scope_node->inner_scope = root;
	new_starting_scope_node->starting_node_ids = starting_halfway_node_context;

	vector<int> possible_inner_layers;
	vector<bool> possible_inner_is_local;
	vector<int> possible_inner_indexes;

	{
		for (int s_index = 0; s_index < root->num_input_states; s_index++) {
			possible_inner_layers.push_back(0);
			possible_inner_is_local.push_back(false);
			possible_inner_indexes.push_back(s_index);
		}
	}
	Scope* curr_scope = root;
	for (int l_index = 0; l_index < (int)starting_halfway_node_context.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[starting_halfway_node_context[l_index]];

		for (int s_index = 0; s_index < curr_scope->num_local_states; s_index++) {
			possible_inner_layers.push_back(l_index);
			possible_inner_is_local.push_back(true);
			possible_inner_indexes.push_back(s_index);
		}

		curr_scope = scope_node->inner_scope;
	}
	{
		for (int s_index = 0; s_index < curr_scope->num_local_states; s_index++) {
			possible_inner_layers.push_back((int)starting_halfway_node_context.size()-1);
			possible_inner_is_local.push_back(true);
			possible_inner_indexes.push_back(s_index);
		}
	}

	vector<map<int, StateStatus>> halfway_inner_input_state_vals(starting_halfway_node_context.size());
	vector<map<int, StateStatus>> halfway_inner_local_state_vals(starting_halfway_node_context.size());

	int num_inputs_to_consider = min(20, (int)possible_inner_layers.size());

	uniform_int_distribution<int> input_type_distribution(0, 3);
	uniform_int_distribution<int> init_distribution(0, 1);
	for (int i_index = 0; i_index < num_inputs_to_consider; i_index++) {
		uniform_int_distribution<int> target_distribution(0, (int)possible_inner_layers.size()-1);
		int rand_target = target_distribution(generator);

		int type = input_type_distribution(generator);
		if (type == 0 && possible_input_scope_depths.size() > 0) {
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
			// it != end()

			int new_state_index = new_num_input_states;
			new_num_input_states++;

			if (possible_inner_is_local[rand_target]) {
				halfway_inner_local_state_vals[possible_inner_layers[rand_target]][possible_inner_indexes[rand_target]] = val_it->second;
			} else {
				// possible_inner_layers[rand_target] == 0
				halfway_inner_input_state_vals[0][possible_inner_indexes[rand_target]] = val_it->second;
			}

			new_starting_scope_node->input_types.push_back(INPUT_TYPE_STATE);
			new_starting_scope_node->input_inner_layers.push_back(possible_inner_layers[rand_target]);
			new_starting_scope_node->input_inner_is_local.push_back(possible_inner_is_local[rand_target]);
			new_starting_scope_node->input_inner_indexes.push_back(possible_inner_indexes[rand_target]);
			new_starting_scope_node->input_outer_is_local.push_back(false);
			new_starting_scope_node->input_outer_indexes.push_back(new_state_index);
			new_starting_scope_node->input_init_vals.push_back(0.0);

			new_sequence->input_types.push_back(INPUT_TYPE_STATE);
			new_sequence->input_inner_indexes.push_back(new_state_index);
			new_sequence->input_scope_depths.push_back(possible_input_scope_depths[rand_input]);
			new_sequence->input_outer_is_local.push_back(possible_input_outer_is_local[rand_input]);
			new_sequence->input_outer_indexes.push_back(possible_input_outer_indexes[rand_input]);
			new_sequence->input_init_vals.push_back(0.0);

			possible_input_scope_depths.erase(possible_input_scope_depths.begin() + rand_input);
			possible_input_outer_is_local.erase(possible_input_outer_is_local.begin() + rand_input);
			possible_input_outer_indexes.erase(possible_input_outer_indexes.begin() + rand_input);
		} else if (type == 1 || type == 2) {
			// constant
			double init_val;
			if (type == 1) {
				init_val = 2*init_distribution(generator)-1;
			} else {
				init_val = 0.0;
			}

			if (possible_inner_is_local[rand_target]) {
				halfway_inner_local_state_vals[possible_inner_layers[rand_target]][possible_inner_indexes[rand_target]] = StateStatus(init_val);
			} else {
				halfway_inner_input_state_vals[possible_inner_layers[rand_target]][possible_inner_indexes[rand_target]] = StateStatus(init_val);
			}

			new_starting_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
			new_starting_scope_node->input_inner_layers.push_back(possible_inner_layers[rand_target]);
			new_starting_scope_node->input_inner_is_local.push_back(possible_inner_is_local[rand_target]);
			new_starting_scope_node->input_inner_indexes.push_back(possible_inner_indexes[rand_target]);
			new_starting_scope_node->input_outer_is_local.push_back(false);
			new_starting_scope_node->input_outer_indexes.push_back(-1);
			new_starting_scope_node->input_init_vals.push_back(init_val);
		} else {
			// none
		}

		possible_inner_layers.erase(possible_inner_layers.begin() + rand_target);
		possible_inner_is_local.erase(possible_inner_is_local.begin() + rand_target);
		possible_inner_indexes.erase(possible_inner_indexes.begin() + rand_target);
	}

	vector<int> starting_node_ids_copy = starting_halfway_node_context;

	vector<ContextLayer> temp_context;
	temp_context.push_back(ContextLayer());

	temp_context.back().scope_id = 0;
	temp_context.back().node_id = -1;

	temp_context.back().input_state_vals = halfway_inner_input_state_vals[0];
	halfway_inner_input_state_vals.erase(halfway_inner_input_state_vals.begin());
	temp_context.back().local_state_vals = halfway_inner_local_state_vals[0];
	halfway_inner_local_state_vals.erase(halfway_inner_local_state_vals.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(root);

	// unused
	int inner_exit_depth = -1;
	int inner_exit_node_id = -1;

	root->activate(starting_halfway_node_context,
				   halfway_inner_input_state_vals,
				   halfway_inner_local_state_vals,
				   problem,
				   temp_context,
				   inner_exit_depth,
				   inner_exit_node_id,
				   run_helper,
				   inner_scope_history);

	delete inner_scope_history;

	if (temp_context.back().input_state_vals.size() > 0) {
		vector<int> possible_output_scope_depths;
		vector<bool> possible_output_outer_is_local;
		vector<int> possible_output_outer_indexes;
		// skip context.back()
		{
			Scope* scope = solution->scopes[context[context.size()-1 - 1].scope_id];
			for (int s_index = 0; s_index < scope->num_input_states; s_index++) {
				possible_output_scope_depths.push_back(1);
				possible_output_outer_is_local.push_back(false);
				possible_output_outer_indexes.push_back(s_index);
			}
			for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
				possible_output_scope_depths.push_back(1);
				possible_output_outer_is_local.push_back(true);
				possible_output_outer_indexes.push_back(s_index);
			}
		}
		for (int c_index = 2; c_index < explore_context_depth; c_index++) {
			Scope* scope = solution->scopes[context[context.size()-1 - c_index].scope_id];
			ScopeNode* scope_node = (ScopeNode*)scope->nodes[context[context.size()-1 - c_index].node_id];

			for (int s_index = 0; s_index < scope->num_input_states; s_index++) {
				bool passed_out = false;
				for (int o_index = 0; o_index < (int)scope_node->output_inner_indexes.size(); o_index++) {
					if (scope_node->output_outer_is_local[o_index] == false
							&& scope_node->output_outer_indexes[o_index] == s_index) {
						passed_out = true;
						break;
					}
				}

				if (!passed_out) {
					possible_output_scope_depths.push_back(c_index);
					possible_output_outer_is_local.push_back(false);
					possible_output_outer_indexes.push_back(s_index);
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
		for (map<int, StateStatus>::iterator it = temp_context.back().input_state_vals.begin();
				it != temp_context.back().input_state_vals.end(); it++) {
			possible_inner_output_ids.push_back(it->first);
		}

		int num_outputs_to_consider = min(10, (int)possible_output_scope_depths.size());

		uniform_int_distribution<int> output_type_distribution(0, 1);
		for (int o_index = 0; o_index < num_outputs_to_consider; o_index++) {
			uniform_int_distribution<int> target_distribution(0, (int)possible_output_scope_depths.size()-1);
			int rand_target = target_distribution(generator);

			int input_index = -1;
			for (int i_index = 0; i_index < (int)new_sequence->input_types.size(); i_index++) {
				if (new_sequence->input_types[i_index] == INPUT_TYPE_STATE) {
					if (new_sequence->input_scope_depths[i_index] == possible_output_scope_depths[rand_target]
							&& new_sequence->input_outer_is_local[i_index] == possible_output_outer_is_local[rand_target]
							&& new_sequence->input_outer_indexes[i_index] == possible_output_outer_indexes[rand_target]) {
						input_index = new_sequence->input_inner_indexes[i_index];
						break;
					}
				}
			}

			if (input_index != -1) {
				if (output_type_distribution(generator) == 0) {
					int input_inner_index;
					for (int i_index = 0; i_index < (int)new_starting_scope_node->input_outer_indexes.size(); i_index++) {
						if (new_starting_scope_node->input_outer_indexes[i_index] == input_index) {
							input_inner_index = new_starting_scope_node->input_inner_indexes[i_index];
							break;
						}
					}

					if (possible_output_outer_is_local[rand_target]) {
						context[context.size()-1 - possible_output_scope_depths[rand_target]]
							.local_state_vals[possible_output_outer_indexes[rand_target]] = temp_context.back().input_state_vals[input_inner_index];
					} else {
						context[context.size()-1 - possible_output_scope_depths[rand_target]]
							.input_state_vals[possible_output_outer_indexes[rand_target]] = temp_context.back().input_state_vals[input_inner_index];
					}

					new_sequence->output_inner_indexes.push_back(input_index);
					new_sequence->output_scope_depths.push_back(possible_output_scope_depths[rand_target]);
					new_sequence->output_outer_is_local.push_back(possible_output_outer_is_local[rand_target]);
					new_sequence->output_outer_indexes.push_back(possible_output_outer_indexes[rand_target]);
				} else {
					// do nothing
				}
			} else {
				/**
				 * - from new_starting_scope_node->input_types == INPUT_TYPE_CONSTANT
				 */
				if (output_type_distribution(generator) == 0) {
					uniform_int_distribution<int> inner_distribution(0, (int)possible_inner_output_ids.size()-1);
					int rand_inner = inner_distribution(generator);

					if (possible_output_outer_is_local[rand_target]) {
						context[context.size()-1 - possible_output_scope_depths[rand_target]]
							.local_state_vals[possible_output_outer_indexes[rand_target]] = temp_context.back().input_state_vals[possible_inner_output_ids[rand_inner]];
					} else {
						context[context.size()-1 - possible_output_scope_depths[rand_target]]
							.input_state_vals[possible_output_outer_indexes[rand_target]] = temp_context.back().input_state_vals[possible_inner_output_ids[rand_inner]];
					}

					int new_state_index = new_num_input_states;
					new_num_input_states++;

					new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(-1);
					new_sequence->input_outer_is_local.push_back(-1);
					new_sequence->input_outer_indexes.push_back(-1);
					new_sequence->input_init_vals.push_back(0.0);

					new_starting_scope_node->output_inner_indexes.push_back(possible_inner_output_ids[rand_inner]);
					new_starting_scope_node->output_outer_is_local.push_back(false);
					new_starting_scope_node->output_outer_indexes.push_back(new_state_index);

					new_sequence->output_inner_indexes.push_back(new_state_index);
					new_sequence->output_scope_depths.push_back(possible_output_scope_depths[rand_target]);
					new_sequence->output_outer_is_local.push_back(possible_output_outer_is_local[rand_target]);
					new_sequence->output_outer_indexes.push_back(possible_output_outer_indexes[rand_target]);

					// can duplicate, so don't remove from possible_inner_output_ids
				} else {
					// do nothing
				}
			}

			possible_output_scope_depths.erase(possible_output_scope_depths.begin() + rand_target);
			possible_output_outer_is_local.erase(possible_output_outer_is_local.begin() + rand_target);
			possible_output_outer_indexes.erase(possible_output_outer_indexes.begin() + rand_target);
		}
	}

	Scope* new_scope = new Scope();

	// don't set id/increment scope_counter until train

	new_scope->num_input_states = new_num_input_states;
	new_scope->num_local_states = 0;

	new_scope->nodes.push_back(new_starting_scope_node);

	new_starting_scope_node->id = 0;
	new_starting_scope_node->next_node_id = -1;

	Scope* parent_scope = solution->scopes[context[context.size() - explore_context_depth].scope_id];
	new_scope->average_score = parent_scope->average_score;
	new_scope->score_variance = parent_scope->score_variance;
	new_scope->average_misguess = parent_scope->average_misguess;
	new_scope->misguess_variance = parent_scope->misguess_variance;

	new_sequence->scope = new_scope;

	return new_sequence;
}
