#include "scope_node.h"

using namespace std;

void ScopeNode::create_sequence_activate(vector<double>& flat_vals,
										 vector<ContextLayer>& context,
										 int target_num_nodes,
										 int& curr_num_nodes,
										 map<Scope*, map<int, int>>& state_mapping,
										 int& new_num_states,
										 vector<AbstractNode*>& new_nodes,
										 RunHelper& run_helper) {
	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<map<int, double>> inner_state_vals(this->starting_node_ids.size());
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			map<int, double> it = context.back().state_vals.find(this->input_ids[i_index]);
			if (it != context.back().state_vals.end()) {
				double val;
				if (this->input_reverse_sign_front[i_index]) {
					val = it->second;
				} else {
					val = -it->second;
				}
				inner_state_vals[this->input_target_layers[i_index]][this->input_target_ids[i_index]] = val;
			}
		} else {
			inner_state_vals[this->input_target_layers[i_index]][this->input_target_ids[i_index]] = this->input_init_vals[i_index];
		}
	}

	// no need to set context.back().node_id

	context.push_back(ContextLayer());

	context.back().scope_id = -1;
	context.back().node_id = -1;

	context.back().state_vals = &(inner_state_vals[0]);

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	vector<vector<double>*> inner_state_vals_copy(this->starting_node_ids.size()-1);
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		inner_state_vals_copy[l_index] = &(inner_state_vals[1+l_index]);
	}

	// currently, starting_node_ids.size() == inner_state_vals_copy.size()+1

	uniform_int_distribution<int> distribution(0, 1);
	if (!inner_scope->is_loop
			&& distribution(generator) == 0) {
		Scope* curr_scope = inner_scope;
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE
					&& this->input_target_layers[i_index] == 0) {
				map<Scope*, map<int, int>>::iterator scope_it = state_mapping.find(curr_scope);
				if (scope_it != state_mapping.end()) {
					map<int, int>::iterator state_it = scope_it->second.find(this->input_target_ids[i_index]);
					if (state_it == scope_it->second.end()) {
						scope_it->second[this->input_target_ids[i_index]] = new_num_states;
						new_num_states++;
					}
				} else {
					state_mapping[curr_scope] = map<int, int>{{this->input_target_ids[i_index], new_num_states}};
					new_num_states++;
				}
			}
		}
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
			curr_scope = solution->scopes[scope_node->inner_scope_id];

			for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
				if (this->input_types[i_index] == INPUT_TYPE_STATE
						&& this->input_target_layers[i_index] == 1+l_index) {
					map<Scope*, map<int, int>>::iterator scope_it = state_mapping.find(curr_scope);
					if (scope_it != state_mapping.end()) {
						map<int, int>::iterator state_it = scope_it->second.find(this->input_target_ids[i_index]);
						if (state_it == scope_it->second.end()) {
							scope_it->second[this->input_target_ids[i_index]] = new_num_states;
							new_num_states++;
						}
					} else {
						state_mapping[curr_scope] = map<int, int>{{this->input_target_ids[i_index], new_num_states}};
						new_num_states++;
					}
				}
			}
		}

		inner_scope->create_sequence_activate(
			starting_node_ids_copy,
			inner_state_vals_copy,
			flat_vals,
			context,
			target_num_nodes,
			curr_num_nodes,
			state_mapping,
			new_num_states,
			new_nodes,
			run_helper);
	} else {
		ScopeNode* new_node = new ScopeNode();
		new_nodes.push_back(new_node);

		new_node->inner_scope_id = this->inner_scope_id;

		new_node->starting_node_ids = this->starting_node_ids;

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
		inner_scope->activate(starting_node_ids_copy,
							  inner_state_vals_copy,
							  flat_vals,
							  context,
							  inner_exit_depth,
							  inner_exit_node_id,
							  run_helper,
							  inner_scope_history);
		delete inner_scope_history;

		new_node->input_types = this->input_types;
		new_node->input_target_layers = this->input_target_layers;
		new_node->input_target_ids = this->input_target_ids;
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				int new_state_id;
				map<Scope*, map<int, int>>::iterator scope_it = state_mapping.find(this->parent);
				if (scope_it != state_mapping.end()) {
					map<int, int>::iterator state_it = scope_it->second.find(this->input_ids[i_index]);
					if (state_it != scope_it->second.end()) {
						new_state_id = state_it->second;
					} else {
						scope_it->second[this->input_ids[i_index]] = new_num_states;
						new_state_id = new_num_states;
						new_num_states++;
					}
				} else {
					state_mapping[this->parent] = map<int, int>{{this->input_ids[i_index], new_num_states}};
					new_state_id = new_num_states;
					new_num_states++;
				}
				new_node->input_ids.push_back(new_state_id);
			} else {
				new_node->input_ids.push_back(-1);
			}
		}
		new_node->input_reverse_sign_front = this->input_reverse_sign_front;
		new_node->input_reverse_sign_back = this->input_reverse_sign_back;
		new_node->input_init_vals = this->input_init_vals;

		curr_num_nodes++;
	}

	// don't worry about score_state

	context.pop_back();

	// no need to set context.back().node_id

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			map<int, double> it = inner_state_vals[this->input_target_layers[i_index]].find(this->input_target_ids[i_index]);
			if (it != inner_state_vals[this->input_target_layers[i_index]].end()) {
				double val;
				if (this->input_reverse_sign_back[i_index]) {
					val = it->second;
				} else {
					val = -it->second;
				}
				context.back().state_vals[this->input_ids[i_index]] = val;
			}
		}
	}
}

void ScopeNode::halfway_create_sequence_activate(
		vector<int>& starting_node_ids,
		vector<map<int, double>*>& starting_state_vals,
		vector<double>& flat_vals,
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		map<Scope*, map<int, int>>& state_mapping,
		int& new_num_states,
		vector<AbstractNode*>& new_nodes,
		RunHelper& run_helper) {
	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	int furthest_matching_layer = 0;
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size(); l_index++) {
		if (l_index >= (int)starting_node_ids.size()
				|| starting_node_ids[l_index] != this->starting_node_ids[l_index]) {
			break;
		} else {
			furthest_matching_layer++;
		}
	}
	for (int i_index = 0; i_index < (int)this->input_ids.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				map<int, double> it = context.back().state_vals.find(this->input_ids[i_index]);
				if (it != context.back().state_vals.end()) {
					double val;
					if (this->input_reverse_sign_front[i_index]) {
						val = it->second;
					} else {
						val = -it->second;
					}
					starting_state_vals[this->input_target_layers[i_index]][this->input_target_ids[i_index]] = val;
				}
			} else {
				starting_state_vals[this->input_target_layers[i_index]][this->input_target_ids[i_index]] = this->input_init_vals[i_index];
			}
		}
	}

	// no need to set context.back().node_id

	context.push_back(ContextLayer());

	context.back().scope_id = -1;
	context.back().node_id = -1;

	context.back().state_vals = starting_state_vals[0];

	vector<vector<double>*> inner_state_vals(starting_state_vals.size()-1);
	for (int l_index = 0; l_index < (int)starting_state_vals.size()-1; l_index++) {
		inner_state_vals[l_index] = starting_state_vals[1+l_index];
	}

	// currently, starting_node_ids.size() == inner_state_vals.size()+1

	uniform_int_distribution<int> distribution(0, 1);
	// inner_scope->is_loop == false
	if (distribution(generator) == 0) {
		Scope* curr_scope = inner_scope;
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			// 0 <= furthest_matching_layer
			if (this->input_types[i_index] == INPUT_TYPE_STATE
					&& this->input_target_layers[i_index] == 0) {
				map<Scope*, map<int, int>>::iterator scope_it = state_mapping.find(curr_scope);
				if (scope_it != state_mapping.end()) {
					map<int, int>::iterator state_it = scope_it->second.find(this->input_target_ids[i_index]);
					if (state_it == scope_it->second.end()) {
						scope_it->second[this->input_target_ids[i_index]] = new_num_states;
						new_num_states++;
					}
				} else {
					state_mapping[curr_scope] = map<int, int>{{this->input_target_ids[i_index], new_num_states}};
					new_num_states++;
				}
			}
		}
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
			curr_scope = solution->scopes[scope_node->inner_scope_id];

			for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
				if (this->input_target_layers[i_index] <= furthest_matching_layer
						&& this->input_types[i_index] == INPUT_TYPE_STATE
						&& this->input_target_layers[i_index] == 1+l_index) {
					map<Scope*, map<int, int>>::iterator scope_it = state_mapping.find(curr_scope);
					if (scope_it != state_mapping.end()) {
						map<int, int>::iterator state_it = scope_it->second.find(this->input_target_ids[i_index]);
						if (state_it == scope_it->second.end()) {
							scope_it->second[this->input_target_ids[i_index]] = new_num_states;
							new_num_states++;
						}
					} else {
						state_mapping[curr_scope] = map<int, int>{{this->input_target_ids[i_index], new_num_states}};
						new_num_states++;
					}
				}
			}
		}

		inner_scope->create_sequence_activate(
			starting_node_ids,
			inner_state_vals,
			flat_vals,
			context,
			target_num_nodes,
			curr_num_nodes,
			state_mapping,
			new_num_states,
			new_nodes,
			run_helper);
	} else {
		ScopeNode* new_node = new ScopeNode();
		new_nodes.push_back(new_node);

		new_node->inner_scope_id = this->inner_scope_id;

		new_node->starting_node_ids = starting_node_ids;

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
		inner_scope->activate(starting_node_ids,
							  inner_state_vals,
							  flat_vals,
							  context,
							  inner_exit_depth,
							  inner_exit_node_id,
							  run_helper,
							  inner_scope_history);
		delete inner_scope_history;

		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_target_layers[i_index] <= furthest_matching_layer) {
				new_node->input_types.push_back(this->input_types[i_index]);
				new_node->input_target_layers.push_back(this->input_target_layers[i_index]);
				new_node->input_target_ids.push_back(this->input_target_ids[i_index]);
				if (this->input_types[i_index] == INPUT_TYPE_STATE) {
					int new_state_id;
					map<Scope*, map<int, int>>::iterator scope_it = state_mapping.find(this->parent);
					if (scope_it != state_mapping.end()) {
						map<int, int>::iterator state_it = scope_it->second.find(this->input_ids[i_index]);
						if (state_it != scope_it->second.end()) {
							new_state_id = state_it->second;
						} else {
							scope_it->second[this->input_ids[i_index]] = new_num_states;
							new_state_id = new_num_states;
							new_num_states++;
						}
					} else {
						state_mapping[this->parent] = map<int, int>{{this->input_ids[i_index], new_num_states}};
						new_state_id = new_num_states;
						new_num_states++;
					}
					new_node->input_ids.push_back(new_state_id);
				} else {
					new_node->input_ids.push_back(-1);
				}
				new_node->input_reverse_sign_front.push_back(this->input_reverse_sign_front[i_index]);
				new_node->input_reverse_sign_back.push_back(this->input_reverse_sign_back[i_index]);
				new_node->input_init_vals.push_back(this->input_init_vals[i_index]);
			}
		}

		curr_num_nodes++;
	}

	// don't worry about score_state

	context.pop_back();

	// no need to set context.back().node_id

	for (int i_index = 0; i_index < (int)this->input_ids.size(); i_index++) {
		if (this->input_target_layers[i_index] <= furthest_matching_layer) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				map<int, double> it = starting_state_vals[this->input_target_layers[i_index]].find(this->input_target_ids[i_index]);
				if (it != starting_state_vals[this->input_target_layers[i_index]].end()) {
					double val;
					if (this->input_reverse_sign_back[i_index]) {
						val = it->second;
					} else {
						val = -it->second;
					}
					context.back().state_vals[this->input_ids[i_index]] = val;
				}
			}
		}
	}
}
