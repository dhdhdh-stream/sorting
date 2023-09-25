#include "scope_node.h"

using namespace std;

void ScopeNode::create_sequence_activate(vector<double>& flat_vals,
										 vector<ContextLayer>& context,
										 int target_num_nodes,
										 int& curr_num_nodes,
										 Sequence* new_sequence,
										 vector<map<int, int>>& state_mappings,
										 int& new_num_states,
										 vector<AbstractNode*>& new_nodes,
										 RunHelper& run_helper) {
	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<map<int, double>> inner_state_vals(this->starting_node_ids.size());
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			map<int, double>::iterator it = context.back().state_vals.find(this->input_outer_ids[i_index]);
			if (it != context.back().state_vals.end()) {
				inner_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = it->second;
			}
		} else {
			inner_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = this->input_init_vals[i_index];
		}
	}

	// no need to set context.back().node_id

	context.push_back(ContextLayer());

	context.back().scope_id = -1;
	context.back().node_id = -1;

	context.back().state_vals = inner_state_vals[0];
	inner_state_vals.erase(inner_state_vals.begin());

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	// currently, starting_node_ids.size() == inner_state_vals_copy.size()+1

	uniform_int_distribution<int> distribution(0, 1);
	if (!inner_scope->is_loop
			&& distribution(generator) == 0) {
		vector<map<int, int>> inner_state_mappings(this->starting_node_ids.size());
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				map<int, int>::iterator it = state_mappings.back().find(this->input_outer_ids[i_index]);
				if (it != state_mappings.back().end()) {
					inner_state_mappings[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = it->second;
				}
			} else {
				int new_state_id;
				inner_state_mappings[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = new_num_states;
				new_state_id = new_num_states;
				new_num_states++;

				new_sequence->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_sequence->input_inner_ids.push_back(new_state_id);
				new_sequence->input_scope_depths.push_back(-1);
				new_sequence->input_outer_ids.push_back(-1);
				new_sequence->input_init_vals.push_back(this->input_init_vals[i_index]);
			}
		}
		/**
		 * - don't worry about output
		 *   - possibilities will be captured recursing inwards then rebuilt anyways
		 */
		state_mappings.push_back(inner_state_mappings[0]);
		inner_state_mappings.erase(inner_state_mappings.begin());

		inner_scope->create_sequence_activate(
			starting_node_ids_copy,
			inner_state_vals,
			inner_state_mappings,
			flat_vals,
			context,
			target_num_nodes,
			curr_num_nodes,
			new_sequence,
			state_mappings,
			new_num_states,
			new_nodes,
			run_helper);

		if (curr_num_nodes < target_num_nodes) {
			state_mappings.pop_back();

			// don't worry about score_state

			for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
				map<int, double>::iterator it = context.back().find(this->output_inner_ids[o_index]);
				if (it != context.back().end()) {
					context[context.size()-2].state_vals[this->output_outer_ids[o_index]] = it->second;
				}
			}

			context.pop_back();

			// no need to set context.back().node_id
		}
	} else {
		ScopeNode* new_node = new ScopeNode();
		new_nodes.push_back(new_node);

		new_node->inner_scope_id = this->inner_scope_id;

		new_node->starting_node_ids = this->starting_node_ids;

		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			if (this->input_types[i_index] == INPUT_TYPE_STATE) {
				map<int, int>::iterator it = state_mappings.back().find(this->input_outer_ids[i_index]);
				if (it != state_mappings.back().end()) {
					new_node->input_types.push_back(INPUT_TYPE_STATE);
					new_node->input_inner_layers.push_back(this->input_inner_layers[i_index]);
					new_node->input_inner_ids.push_back(this->input_inner_ids[i_index]);
					new_node->input_outer_ids.push_back(it->second);
					new_node->input_init_vals.push_back(0.0);
				}
				/**
				 * - if not found, then means never initialized in new sequence, and can be removed
				 */
			} else {
				new_node->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_node->input_inner_layers.push_back(this->input_inner_layers[i_index]);
				new_node->input_inner_ids.push_back(this->input_inner_ids[i_index]);
				new_node->input_outer_ids.push_back(-1);
				new_node->input_init_vals.push_back(this->input_init_vals[i_index]);
			}
		}

		for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
			map<int, int>::iterator it = state_mappings.back().find(this->output_outer_ids[o_index]);
			if (it == state_mappings.back().end()) {
				it = state_mappings.back().insert({this->output_outer_ids[o_index], new_num_states}).first;
				new_num_states++;
			}
			new_node->output_inner_ids.push_back(this->output_inner_ids[o_index]);
			new_node->output_outer_ids.push_back(it->second);
		}

		curr_num_nodes++;

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
		inner_scope->activate(starting_node_ids_copy,
							  inner_state_vals,
							  flat_vals,
							  context,
							  inner_exit_depth,
							  inner_exit_node_id,
							  run_helper,
							  inner_scope_history);
		delete inner_scope_history;

		// don't worry about score_state

		for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
			map<int, double>::iterator it = context.back().find(this->output_inner_ids[o_index]);
			if (it != context.back().end()) {
				context[context.size()-2].state_vals[this->output_outer_ids[o_index]] = it->second;
			}
		}

		context.pop_back();

		// no need to set context.back().node_id
	}
}

void ScopeNode::halfway_create_sequence_activate(
		vector<int>& starting_node_ids,
		vector<map<int, double>>& starting_state_vals,
		vector<map<int, int>>& starting_state_mappings,
		vector<double>& flat_vals,
		vector<ContextLayer>& context,
		int target_num_nodes,
		int& curr_num_nodes,
		Sequence* new_sequence,
		vector<map<int, int>>& state_mappings,
		int& new_num_states,
		vector<AbstractNode*>& new_nodes,
		RunHelper& run_helper) {
	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	// no need to set context.back().node_id

	context.push_back(ContextLayer());

	context.back().scope_id = -1;
	context.back().node_id = -1;

	context.back().state_vals = starting_state_vals[0];
	starting_state_vals.erase(starting_state_vals.begin());

	// currently, starting_node_ids.size() == starting_state_vals.size()+1

	uniform_int_distribution<int> distribution(0, 1);
	// inner_scope->is_loop == false
	if (distribution(generator) == 0) {
		state_mappings.push_back(starting_state_mappings[0]);
		starting_state_mappings.erase(starting_state_mappings.begin());

		inner_scope->create_sequence_activate(
			starting_node_ids,
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

		if (curr_num_nodes < target_num_nodes) {
			state_mappings.pop_back();

			// don't worry about score_state

			for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
				map<int, double>::iterator it = context.back().find(this->output_inner_ids[o_index]);
				if (it != context.back().end()) {
					context[context.size()-2].state_vals[this->output_outer_ids[o_index]] = it->second;
				}
			}

			context.pop_back();

			// no need to set context.back().node_id
		}
	} else {
		ScopeNode* new_node = new ScopeNode();
		new_nodes.push_back(new_node);

		new_node->inner_scope_id = this->inner_scope_id;

		new_node->starting_node_ids = starting_node_ids;

		for (int l_index = 0; l_index < (int)starting_state_mappings.size(); l_index++) {
			for (map<int, int>::iterator it = starting_state_mappings[l_index].begin();
					it != starting_state_mappings[l_index].end(); it++) {
				new_node->input_types.push_back(INPUT_TYPE_STATE);
				new_node->input_inner_layers.push_back(l_index);
				new_node->input_inner_ids.push_back(it->first);
				new_node->input_outer_ids.push_back(it->second);
				new_node->input_init_vals.push_back(0.0);
			}
		}

		for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
			map<int, int>::iterator it = state_mappings.back().find(this->output_outer_ids[o_index]);
			if (it == state_mappings.back().end()) {
				it = state_mappings.back().insert({this->output_outer_ids[o_index], new_num_states}).first;
				new_num_states++;
			}
			new_node->output_inner_ids.push_back(this->output_inner_ids[o_index]);
			new_node->output_outer_ids.push_back(it->second);
		}

		curr_num_nodes++;

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
		inner_scope->activate(starting_node_ids,
							  starting_state_vals,
							  flat_vals,
							  context,
							  inner_exit_depth,
							  inner_exit_node_id,
							  run_helper,
							  inner_scope_history);
		delete inner_scope_history;

		// don't worry about score_state

		for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
			map<int, double>::iterator it = context.back().find(this->output_inner_ids[o_index]);
			if (it != context.back().end()) {
				context[context.size()-2].state_vals[this->output_outer_ids[o_index]] = it->second;
			}
		}

		context.pop_back();

		// no need to set context.back().node_id
	}
}
