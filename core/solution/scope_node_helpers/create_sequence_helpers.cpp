#include "scope_node.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "sequence.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ScopeNode::create_sequence_activate(Problem& problem,
										 vector<ContextLayer>& context,
										 int target_num_nodes,
										 int& curr_num_nodes,
										 Sequence* new_sequence,
										 vector<map<pair<bool,int>, int>>& state_mappings,
										 int& new_num_input_states,
										 vector<AbstractNode*>& new_nodes,
										 RunHelper& run_helper) {
	vector<map<int, StateStatus>> inner_input_state_vals(this->starting_node_ids.size());
	vector<map<int, StateStatus>> inner_local_state_vals(this->starting_node_ids.size());
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				if (this->input_inner_is_local[i_index]) {
					inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = state_status;
				} else {
					inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = state_status;
				}
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					if (this->input_inner_is_local[i_index]) {
						inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = it->second;
					} else {
						inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = it->second;
					}
				}
			}
		} else {
			if (this->input_inner_is_local[i_index]) {
				inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
			} else {
				inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
			}
		}
	}

	// no need to set context.back().node_id

	context.push_back(ContextLayer());

	context.back().scope_id = -1;
	context.back().node_id = -1;

	context.back().input_state_vals = inner_input_state_vals[0];
	inner_input_state_vals.erase(inner_input_state_vals.begin());
	context.back().local_state_vals = inner_local_state_vals[0];
	inner_local_state_vals.erase(inner_local_state_vals.begin());

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	// currently, starting_node_ids.size() == inner_state_vals_copy.size()+1

	uniform_int_distribution<int> distribution(0, 1);
	// TODO: check inner_scope->is_loop
	if (distribution(generator) == 0) {
		vector<map<pair<bool,int>, int>> inner_state_mappings(this->starting_node_ids.size());
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				if (this->input_outer_is_local[i_index]) {
					map<pair<bool,int>, int>::iterator it = state_mappings.back()
						.find({true, this->input_outer_indexes[i_index]});
					if (it != state_mappings.back().end()) {
						inner_state_mappings[this->input_inner_layers[i_index]]
							[{this->input_inner_is_local[i_index], this->input_inner_indexes[i_index]}] = it->second;
					} else {
						int new_state_index;
						state_mappings.back()[{true, this->input_outer_indexes[i_index]}] = new_num_input_states;
						new_state_index = new_num_input_states;
						new_num_input_states++;

						inner_state_mappings[this->input_inner_layers[i_index]]
							[{this->input_inner_is_local[i_index], this->input_inner_indexes[i_index]}] = new_state_index;

						new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
						new_sequence->input_inner_indexes.push_back(new_state_index);
						new_sequence->input_scope_depths.push_back(-1);
						new_sequence->input_outer_is_local.push_back(-1);
						new_sequence->input_outer_indexes.push_back(-1);
						new_sequence->input_init_vals.push_back(0.0);
					}
				} else {
					map<pair<bool,int>, int>::iterator it = state_mappings.back()
						.find({false, this->input_outer_indexes[i_index]});
					if (it != state_mappings.back().end()) {
						inner_state_mappings[this->input_inner_layers[i_index]]
							[{this->input_inner_is_local[i_index], this->input_inner_indexes[i_index]}] = it->second;
					}
				}
			} else {
				int new_state_index;
				inner_state_mappings[this->input_inner_layers[i_index]]
					[{this->input_inner_is_local[i_index], this->input_inner_indexes[i_index]}] = new_num_input_states;
				new_state_index = new_num_input_states;
				new_num_input_states++;

				new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_sequence->input_inner_indexes.push_back(new_state_index);
				new_sequence->input_scope_depths.push_back(-1);
				new_sequence->input_outer_is_local.push_back(-1);
				new_sequence->input_outer_indexes.push_back(-1);
				new_sequence->input_init_vals.push_back(this->input_init_vals[i_index]);
			}
		}
		/**
		 * - don't worry about output
		 *   - simply stop (i.e., don't continue outside) even if curr_num_nodes < target_num_nodes
		 */
		state_mappings.push_back(inner_state_mappings[0]);
		inner_state_mappings.erase(inner_state_mappings.begin());

		this->inner_scope->create_sequence_activate(
			starting_node_ids_copy,
			inner_input_state_vals,
			inner_local_state_vals,
			inner_state_mappings,
			problem,
			context,
			target_num_nodes,
			curr_num_nodes,
			new_sequence,
			state_mappings,
			new_num_input_states,
			new_nodes,
			run_helper);

		if (curr_num_nodes < target_num_nodes) {
			target_num_nodes = curr_num_nodes;
			// to trigger exit
		}
	} else {
		ScopeNode* new_node = new ScopeNode();
		new_nodes.push_back(new_node);

		new_node->inner_scope = this->inner_scope;

		new_node->starting_node_ids = this->starting_node_ids;

		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				map<pair<bool,int>, int>::iterator it = state_mappings.back()
					.find({this->input_outer_is_local[i_index], this->input_outer_indexes[i_index]});
				if (it != state_mappings.back().end()) {
					new_node->input_types.push_back(INPUT_TYPE_STATE);
					new_node->input_inner_layers.push_back(this->input_inner_layers[i_index]);
					new_node->input_inner_is_local.push_back(this->input_inner_is_local[i_index]);
					new_node->input_inner_indexes.push_back(this->input_inner_indexes[i_index]);
					new_node->input_outer_is_local.push_back(false);
					new_node->input_outer_indexes.push_back(it->second);
					new_node->input_init_vals.push_back(0.0);
				}
				/**
				 * - if not found, then means never initialized in new sequence, and can be removed
				 */
			} else {
				new_node->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_node->input_inner_layers.push_back(this->input_inner_layers[i_index]);
				new_node->input_inner_is_local.push_back(this->input_inner_is_local[i_index]);
				new_node->input_inner_indexes.push_back(this->input_inner_indexes[i_index]);
				new_node->input_outer_is_local.push_back(false);
				new_node->input_outer_indexes.push_back(-1);
				new_node->input_init_vals.push_back(this->input_init_vals[i_index]);
			}
		}

		// unused
		int inner_exit_depth = -1;
		int inner_exit_node_id = -1;

		ScopeHistory* inner_scope_history = new ScopeHistory(this->inner_scope);
		this->inner_scope->activate(starting_node_ids_copy,
									inner_input_state_vals,
									inner_local_state_vals,
									problem,
									context,
									inner_exit_depth,
									inner_exit_node_id,
									run_helper,
									inner_scope_history);
		delete inner_scope_history;

		for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
			map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
			if (inner_it != context.back().input_state_vals.end()) {
				if (this->output_outer_is_local[o_index]) {
					context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
				} else {
					map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
					if (outer_it != context[context.size()-2].input_state_vals.end()) {
						outer_it->second = inner_it->second;
					}
				}
			}
		}

		for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
			if (this->output_outer_is_local[o_index]) {
				new_node->output_inner_indexes.push_back(this->output_inner_indexes[o_index]);
				new_node->output_outer_is_local.push_back(false);
				map<pair<bool,int>, int>::iterator new_state_it
					= state_mappings.back().find({true, this->output_outer_indexes[o_index]});
				if (new_state_it != state_mappings.back().end()) {
					new_node->output_outer_indexes.push_back(new_state_it->second);
				} else {
					int new_state_index;
					state_mappings.back()[{true, this->output_outer_indexes[o_index]}] = new_num_input_states;
					new_state_index = new_num_input_states;
					new_num_input_states++;

					new_node->output_outer_indexes.push_back(new_state_index);

					new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(-1);
					new_sequence->input_outer_is_local.push_back(-1);
					new_sequence->input_outer_indexes.push_back(-1);
					new_sequence->input_init_vals.push_back(0.0);
				}
			} else {
				map<pair<bool,int>, int>::iterator new_state_it
					= state_mappings.back().find({false, this->output_outer_indexes[o_index]});
				if (new_state_it != state_mappings.back().end()) {
					new_node->output_inner_indexes.push_back(this->output_inner_indexes[o_index]);
					new_node->output_outer_is_local.push_back(false);
					new_node->output_outer_indexes.push_back(new_state_it->second);
				}
			}
		}

		bool is_early_exit;
		if (inner_exit_depth == -1) {
			is_early_exit = false;
		} else {
			is_early_exit = true;
		}

		map<int, StateStatus> obs_snapshots;
		if (!is_early_exit) {
			obs_snapshots = context.back().local_state_vals;
		}

		context.pop_back();

		// no need to set context.back().node_id

		if (!is_early_exit) {
			for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
				map<int, StateStatus>::iterator obs_it = obs_snapshots.find(this->state_obs_indexes[n_index]);
				if (obs_it != obs_snapshots.end()) {
					if (this->state_is_local[n_index]) {
						map<int, StateStatus>::iterator state_it = context.back().local_state_vals.find(this->state_indexes[n_index]);
						if (state_it == context.back().local_state_vals.end()) {
							state_it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
						}
						StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
						state_network->activate(obs_it->second.val,
												state_it->second);
					} else {
						map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_indexes[n_index]);
						if (state_it != context.back().input_state_vals.end()) {
							StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
							state_network->activate(obs_it->second.val,
													state_it->second);
						}
					}
				}
			}
		}

		for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
			if (this->state_is_local[n_index]) {
				new_node->state_is_local.push_back(false);
				map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back().find({true, this->state_indexes[n_index]});
				if (new_state_it != state_mappings.back().end()) {
					new_node->state_indexes.push_back(new_state_it->second);
				} else {
					int new_state_index;
					state_mappings.back()[{true, this->state_indexes[n_index]}] = new_num_input_states;
					new_state_index = new_num_input_states;
					new_num_input_states++;

					new_node->state_indexes.push_back(new_state_index);

					new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(-1);
					new_sequence->input_outer_is_local.push_back(-1);
					new_sequence->input_outer_indexes.push_back(-1);
					new_sequence->input_init_vals.push_back(0.0);
				}
				new_node->state_defs.push_back(this->state_defs[n_index]);
				new_node->state_network_indexes.push_back(this->state_network_indexes[n_index]);
			} else {
				map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back().find({false, this->state_indexes[n_index]});
				if (new_state_it != state_mappings.back().end()) {
					new_node->state_is_local.push_back(false);
					new_node->state_indexes.push_back(new_state_it->second);
					new_node->state_obs_indexes.push_back(this->state_obs_indexes[n_index]);
					new_node->state_defs.push_back(this->state_defs[n_index]);
					new_node->state_network_indexes.push_back(this->state_network_indexes[n_index]);
				}
			}
		}

		// don't worry about score_state

		curr_num_nodes++;
	}
}

void ScopeNode::halfway_create_sequence_activate(
		vector<int>& starting_node_ids,
		vector<map<int, StateStatus>>& starting_input_state_vals,
		vector<map<int, StateStatus>>& starting_local_state_vals,
		vector<map<pair<bool,int>, int>>& starting_state_mappings,
		Problem& problem,
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		Sequence* new_sequence,
		vector<map<pair<bool,int>, int>>& state_mappings,
		int& new_num_input_states,
		vector<AbstractNode*>& new_nodes,
		RunHelper& run_helper) {
	context.push_back(ContextLayer());

	context.back().scope_id = -1;
	context.back().node_id = -1;

	context.back().input_state_vals = starting_input_state_vals[0];
	starting_input_state_vals.erase(starting_input_state_vals.begin());
	context.back().local_state_vals = starting_local_state_vals[0];
	starting_local_state_vals.erase(starting_local_state_vals.begin());

	// currently, starting_node_ids.size() == starting_state_vals.size()+1

	uniform_int_distribution<int> distribution(0, 1);
	// inner_scope->is_loop == false
	if (distribution(generator) == 0) {
		state_mappings.push_back(starting_state_mappings[0]);
		starting_state_mappings.erase(starting_state_mappings.begin());

		this->inner_scope->create_sequence_activate(
			starting_node_ids,
			starting_input_state_vals,
			starting_local_state_vals,
			starting_state_mappings,
			problem,
			context,
			target_num_nodes,
			curr_num_nodes,
			new_sequence,
			state_mappings,
			new_num_input_states,
			new_nodes,
			run_helper);

		if (curr_num_nodes < target_num_nodes) {
			target_num_nodes = curr_num_nodes;
			// to trigger exit
		}
	} else {
		ScopeNode* new_node = new ScopeNode();
		new_nodes.push_back(new_node);

		new_node->inner_scope = this->inner_scope;

		new_node->starting_node_ids = starting_node_ids;

		for (int l_index = 0; l_index < (int)starting_state_mappings.size(); l_index++) {
			for (map<pair<bool,int>, int>::iterator it = starting_state_mappings[l_index].begin();
					it != starting_state_mappings[l_index].end(); it++) {
				new_node->input_types.push_back(INPUT_TYPE_STATE);
				new_node->input_inner_layers.push_back(l_index);
				new_node->input_inner_is_local.push_back(it->first.first);
				new_node->input_inner_indexes.push_back(it->first.second);
				new_node->input_outer_is_local.push_back(false);
				new_node->input_outer_indexes.push_back(it->second);
				new_node->input_init_vals.push_back(0.0);
			}
		}

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeHistory* inner_scope_history = new ScopeHistory(this->inner_scope);
		this->inner_scope->activate(starting_node_ids,
									starting_input_state_vals,
									starting_local_state_vals,
									problem,
									context,
									inner_exit_depth,
									inner_exit_node_id,
									run_helper,
									inner_scope_history);
		delete inner_scope_history;

		for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
			map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
			if (inner_it != context.back().input_state_vals.end()) {
				if (this->output_outer_is_local[o_index]) {
					context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
				} else {
					map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
					if (outer_it != context[context.size()-2].input_state_vals.end()) {
						outer_it->second = inner_it->second;
					}
				}
			}
		}

		for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
			if (this->output_outer_is_local[o_index]) {
				new_node->output_inner_indexes.push_back(this->output_inner_indexes[o_index]);
				new_node->output_outer_is_local.push_back(false);
				map<pair<bool,int>, int>::iterator new_state_it
					= state_mappings[state_mappings.size()-2].find({true, this->output_outer_indexes[o_index]});
				if (new_state_it != state_mappings[state_mappings.size()-2].end()) {
					new_node->output_outer_indexes.push_back(new_state_it->second);
				} else {
					int new_state_index;
					state_mappings[state_mappings.size()-2][{true, this->output_outer_indexes[o_index]}] = new_num_input_states;
					new_state_index = new_num_input_states;
					new_num_input_states++;

					new_node->output_outer_indexes.push_back(new_state_index);

					new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(-1);
					new_sequence->input_outer_is_local.push_back(-1);
					new_sequence->input_outer_indexes.push_back(-1);
					new_sequence->input_init_vals.push_back(0.0);
				}
			} else {
				map<pair<bool,int>, int>::iterator new_state_it
					= state_mappings[state_mappings.size()-2].find({false, this->output_outer_indexes[o_index]});
				if (new_state_it != state_mappings[state_mappings.size()-2].end()) {
					new_node->output_inner_indexes.push_back(this->output_inner_indexes[o_index]);
					new_node->output_outer_is_local.push_back(false);
					new_node->output_outer_indexes.push_back(new_state_it->second);
				}
			}
		}

		bool is_early_exit;
		if (inner_exit_depth == -1) {
			is_early_exit = false;
		} else {
			is_early_exit = true;
		}

		map<int, StateStatus> obs_snapshots;
		if (!is_early_exit) {
			obs_snapshots = context.back().local_state_vals;
		}

		context.pop_back();

		if (!is_early_exit) {
			for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
				map<int, StateStatus>::iterator obs_it = obs_snapshots.find(this->state_obs_indexes[n_index]);
				if (obs_it != obs_snapshots.end()) {
					if (this->state_is_local[n_index]) {
						map<int, StateStatus>::iterator state_it = context.back().local_state_vals.find(this->state_indexes[n_index]);
						if (state_it == context.back().local_state_vals.end()) {
							state_it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
						}
						StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
						state_network->activate(obs_it->second.val,
												state_it->second);
					} else {
						map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_indexes[n_index]);
						if (state_it != context.back().input_state_vals.end()) {
							StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
							state_network->activate(obs_it->second.val,
													state_it->second);
						}
					}
				}
			}
		}

		for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
			if (this->state_is_local[n_index]) {
				new_node->state_is_local.push_back(false);
				map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back().find({true, this->state_indexes[n_index]});
				if (new_state_it != state_mappings.back().end()) {
					new_node->state_indexes.push_back(new_state_it->second);
				} else {
					int new_state_index;
					state_mappings.back()[{true, this->state_indexes[n_index]}] = new_num_input_states;
					new_state_index = new_num_input_states;
					new_num_input_states++;

					new_node->state_indexes.push_back(new_state_index);

					new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_sequence->input_inner_indexes.push_back(new_state_index);
					new_sequence->input_scope_depths.push_back(-1);
					new_sequence->input_outer_is_local.push_back(-1);
					new_sequence->input_outer_indexes.push_back(-1);
					new_sequence->input_init_vals.push_back(0.0);
				}
				new_node->state_defs.push_back(this->state_defs[n_index]);
				new_node->state_network_indexes.push_back(this->state_network_indexes[n_index]);
			} else {
				map<pair<bool,int>, int>::iterator new_state_it = state_mappings.back().find({false, this->state_indexes[n_index]});
				if (new_state_it != state_mappings.back().end()) {
					new_node->state_is_local.push_back(false);
					new_node->state_indexes.push_back(new_state_it->second);
					new_node->state_obs_indexes.push_back(this->state_obs_indexes[n_index]);
					new_node->state_defs.push_back(this->state_defs[n_index]);
					new_node->state_network_indexes.push_back(this->state_network_indexes[n_index]);
				}
			}
		}

		// don't worry about score_state

		curr_num_nodes++;
	}
}

// for create_sequence()
void ScopeNode::simple_halfway_activate(vector<int>& starting_node_ids,
										vector<map<int, StateStatus>>& starting_input_state_vals,
										vector<map<int, StateStatus>>& starting_local_state_vals,
										Problem& problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper) {
	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope->id;
	context.back().node_id = -1;

	context.back().input_state_vals = starting_input_state_vals[0];
	starting_input_state_vals.erase(starting_input_state_vals.begin());
	context.back().local_state_vals = starting_local_state_vals[0];
	starting_local_state_vals.erase(starting_local_state_vals.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(this->inner_scope);
	// no need to set context.back().scope_history

	// currently, starting_node_ids.size() == starting_state_vals.size()+1

	// unused
	int inner_exit_depth = -1;
	int inner_exit_node_id = -1;

	this->inner_scope->activate(starting_node_ids,
								starting_input_state_vals,
								starting_local_state_vals,
								problem,
								context,
								inner_exit_depth,
								inner_exit_node_id,
								run_helper,
								inner_scope_history);

	delete inner_scope_history;

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
			} else {
				map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2].input_state_vals.end()) {
					outer_it->second = inner_it->second;
				}
			}
		}
	}

	map<int, StateStatus> obs_snapshots = context.back().local_state_vals;

	context.pop_back();

	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		map<int, StateStatus>::iterator obs_it = obs_snapshots.find(this->state_obs_indexes[n_index]);
		if (obs_it != obs_snapshots.end()) {
			if (this->state_is_local[n_index]) {
				map<int, StateStatus>::iterator state_it = context.back().local_state_vals.find(this->state_indexes[n_index]);
				if (state_it == context.back().local_state_vals.end()) {
					state_it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				state_network->activate(obs_it->second.val,
										state_it->second);
			} else {
				map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_indexes[n_index]);
				if (state_it != context.back().input_state_vals.end()) {
					StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
					state_network->activate(obs_it->second.val,
											state_it->second);
				}
			}
		}
	}
}
