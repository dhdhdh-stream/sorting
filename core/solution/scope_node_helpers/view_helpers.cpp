#include "scope_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "scope.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ScopeNode::view_activate(AbstractNode*& curr_node,
							  Problem& problem,
							  vector<ContextLayer>& context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  RunHelper& run_helper) {
	cout << "scope node #" << this->id << endl;

	vector<map<int, StateStatus>> inner_input_state_vals(this->starting_nodes.size());
	vector<map<int, StateStatus>> inner_local_state_vals(this->starting_nodes.size());
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				if (this->input_inner_is_local[i_index]) {
					cout << "i" << this->input_inner_layers[i_index] << " local #" << this->input_inner_indexes[i_index] << ": " << state_status.val << endl;
					inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = state_status;
				} else {
					cout << "i" << this->input_inner_layers[i_index] << " inner #" << this->input_inner_indexes[i_index] << ": " << state_status.val << endl;
					inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = state_status;
				}
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					if (this->input_inner_is_local[i_index]) {
						cout << "i" << this->input_inner_layers[i_index] << " local #" << this->input_inner_indexes[i_index] << ": " << it->second.val << endl;
						inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = it->second;
					} else {
						cout << "i" << this->input_inner_layers[i_index] << " inner #" << this->input_inner_indexes[i_index] << ": " << it->second.val << endl;
						inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = it->second;
					}
				}
			}
		} else {
			if (this->input_inner_is_local[i_index]) {
				inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
				cout << "i" << this->input_inner_layers[i_index] << " local #" << this->input_inner_indexes[i_index] << ": " << this->input_init_vals[i_index] << endl;
			} else {
				inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
				cout << "i" << this->input_inner_layers[i_index] << " inner #" << this->input_inner_indexes[i_index] << ": " << this->input_init_vals[i_index] << endl;
			}
		}
	}

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope->id;
	context.back().node_id = -1;

	context.back().input_state_vals = inner_input_state_vals[0];
	inner_input_state_vals.erase(inner_input_state_vals.begin());
	context.back().local_state_vals = inner_local_state_vals[0];
	inner_local_state_vals.erase(inner_local_state_vals.begin());

	vector<AbstractNode*> starting_nodes_copy = this->starting_nodes;

	// currently, starting_nodes.size() == inner_state_vals.size()+1

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->inner_scope->view_activate(starting_nodes_copy,
									 inner_input_state_vals,
									 inner_local_state_vals,
									 problem,
									 context,
									 inner_exit_depth,
									 inner_exit_node,
									 run_helper);

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
				cout << "o local #" << this->output_outer_indexes[o_index] << ": " << inner_it->second.val << endl;
			} else {
				map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2].input_state_vals.end()) {
					outer_it->second = inner_it->second;
					cout << "o input #" << this->output_outer_indexes[o_index] << ": " << inner_it->second.val << endl;
				}
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

	context.back().node_id = -1;

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
					cout << "local state #" << this->state_indexes[n_index] << ": " << state_it->second.val << endl;
				} else {
					map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_indexes[n_index]);
					if (state_it != context.back().input_state_vals.end()) {
						StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
						state_network->activate(obs_it->second.val,
												state_it->second);
						cout << "input state #" << this->state_indexes[n_index] << ": " << state_it->second.val << endl;
					}
				}
			}
		}

		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}

	cout << endl;
}

void ScopeNode::halfway_view_activate(vector<AbstractNode*>& starting_nodes,
									  vector<map<int, StateStatus>>& starting_input_state_vals,
									  vector<map<int, StateStatus>>& starting_local_state_vals,
									  AbstractNode*& curr_node,
									  Problem& problem,
									  vector<ContextLayer>& context,
									  int& exit_depth,
									  AbstractNode*& exit_node,
									  RunHelper& run_helper) {
	cout << "scope node #" << this->id << endl;

	cout << "is_halfway" << endl;

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_inner_layers[i_index] == 0
					&& !this->input_inner_is_local[i_index]) {
				if (this->input_outer_is_local[i_index]) {
					StateStatus state_status;
					map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
					if (it != context.back().local_state_vals.end()) {
						state_status = it->second;
					}
					starting_input_state_vals[0][this->input_inner_indexes[i_index]] = state_status;
					cout << "i " << " inner #" << this->input_inner_indexes[i_index] << ": " << state_status.val << endl;
				} else {
					map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
					if (it != context.back().input_state_vals.end()) {
						starting_input_state_vals[0][this->input_inner_indexes[i_index]] = it->second;
						cout << "i " << " inner #" << this->input_inner_indexes[i_index] << ": " << it->second.val << endl;
					}
				}
			}
		}
	}

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope->id;
	context.back().node_id = -1;

	context.back().input_state_vals = starting_input_state_vals[0];
	starting_input_state_vals.erase(starting_input_state_vals.begin());
	context.back().local_state_vals = starting_local_state_vals[0];
	starting_local_state_vals.erase(starting_local_state_vals.begin());

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->inner_scope->view_activate(starting_nodes,
									 starting_input_state_vals,
									 starting_local_state_vals,
									 problem,
									 context,
									 inner_exit_depth,
									 inner_exit_node,
									 run_helper);

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
				cout << "o local #" << this->output_outer_indexes[o_index] << ": " << inner_it->second.val << endl;
			} else {
				map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2].input_state_vals.end()) {
					outer_it->second = inner_it->second;
					cout << "o input #" << this->output_outer_indexes[o_index] << ": " << inner_it->second.val << endl;
				}
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

	context.back().node_id = -1;

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
					cout << "local state #" << this->state_indexes[n_index] << ": " << state_it->second.val << endl;
				} else {
					map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_indexes[n_index]);
					if (state_it != context.back().input_state_vals.end()) {
						StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
						state_network->activate(obs_it->second.val,
												state_it->second);
						cout << "input state #" << this->state_indexes[n_index] << ": " << state_it->second.val << endl;
					}
				}
			}
		}

		curr_node = this->next_node;
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
