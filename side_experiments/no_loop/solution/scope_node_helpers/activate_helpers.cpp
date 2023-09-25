#include "scope_node.h"

using namespace std;

void ScopeNode::activate(vector<double>& flat_vals,
						 vector<ContextLayer>& context,
						 int& inner_exit_depth,
						 int& inner_exit_node_id,
						 RunHelper& run_helper,
						 vector<AbstractNodeHistory*>& node_histories) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	node_histories.push_back(history);

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	vector<map<int, StateStatus>> inner_input_state_vals(this->starting_node_ids.size());
	vector<map<int, StateStatus>> inner_local_state_vals(this->starting_node_ids.size());
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_ids[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				if (this->input_inner_is_local[i_index]) {
					inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = state_status;
				} else {
					inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = state_status;
				}
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_ids[i_index]);
				if (it != context.back().input_state_vals.end()) {
					if (this->input_inner_is_local[i_index]) {
						inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = it->second;
					} else {
						inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = it->second;
					}
				}
			}
		} else {
			if (this->input_inner_is_local[i_index]) {
				inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = StateStatus(this->input_init_vals[i_index]);
			} else {
				inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = StateStatus(this->input_init_vals[i_index]);
			}
		}
	}

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().input_state_vals = inner_input_state_vals[0];
	inner_input_state_vals.erase(inner_input_state_vals.begin());
	context.back().local_state_vals = inner_local_state_vals[0];
	inner_local_state_vals.erase(inner_local_state_vals.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	// currently, starting_node_ids.size() == inner_state_vals.size()+1

	inner_scope->activate(starting_node_ids_copy,
						  inner_input_state_vals,
						  inner_local_state_vals,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
		map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->output_inner_ids[o_index]);
		if (it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2].local_state_vals[this->output_outer_ids[o_index]] = it->second;
			} else {
				context[context.size()-2].input_state_vals[this->output_outer_ids[o_index]] = it->second;
			}
		}
	}

	history->obs_snapshots = context.back().local_state_vals;

	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->state_obs_indexes[n_index]);
		if (obs_it != history->obs_snapshots.end()) {
			if (this->state_is_local[n_index]) {
				map<int, StateStatus>::iterator state_it = context.back().local_state_vals.find(this->state_ids[n_index]);
				if (state_it == context.back().local_state_vals.end()) {
					state_it = context.back().local_state_vals.insert({this->state_ids[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				state_network->activate(obs_it->second,
										state_it->second);
			} else {
				map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_ids[n_index]);
				if (state_it != context.back().input_state_vals.end()) {
					StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
					state_network->activate(obs_it->second,
											state_it->second);
				}
			}
		}
	}

	for (int n_index = 0; n_index < (int)this->score_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->score_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->score_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->score_state_scope_contexts[n_index][c_index] != context[context.size()-this->score_state_scope_contexts[n_index].size()+c_index].scope_id
						|| this->score_state_node_contexts[n_index][c_index] != context[context.size()-this->score_state_scope_contexts[n_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->score_state_obs_indexes[n_index]);
			if (obs_it != history->obs_snapshots.end()) {
				map<State*, StateStatus>::iterator state_it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.find(this->score_state_defs[n_index]);
				if (state_it == context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.end()) {
					state_it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.insert({this->score_state_defs[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->score_state_defs[n_index]->networks[this->score_state_network_indexes[n_index]];
				state_network->activate(obs_it->second,
										state_it->second);
			}
		}
	}

	context.pop_back();

	context.back().node_id = -1;
}

void ScopeNode::halfway_activate(vector<int>& starting_node_ids,
								 vector<map<int, StateStatus>>& starting_input_state_vals,
								 vector<map<int, StateStatus>>& starting_local_state_vals,
								 vector<double>& flat_vals,
								 vector<ContextLayer>& context,
								 int& inner_exit_depth,
								 int& inner_exit_node_id,
								 RunHelper& run_helper,
								 vector<AbstractNodeHistory*>& node_histories) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	node_histories.push_back(history);

	history->is_halfway = true;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().input_state_vals = starting_input_state_vals[0];
	starting_input_state_vals.erase(starting_input_state_vals.begin());
	context.back().local_state_vals = starting_local_state_vals[0];
	starting_local_state_vals.erase(starting_local_state_vals.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	// currently, starting_node_ids.size() == starting_state_vals.size()+1

	inner_scope->activate(starting_node_ids,
						  starting_input_state_vals,
						  starting_local_state_vals,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
		map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->output_inner_ids[o_index]);
		if (it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2].local_state_vals[this->output_outer_ids[o_index]] = it->second;
			} else {
				context[context.size()-2].input_state_vals[this->output_outer_ids[o_index]] = it->second;
			}
		}
	}

	history->obs_snapshots = context.back().local_state_vals;

	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->state_obs_indexes[n_index]);
		if (obs_it != history->obs_snapshots.end()) {
			if (this->state_is_local[n_index]) {
				map<int, StateStatus>::iterator state_it = context.back().local_state_vals.find(this->state_ids[n_index]);
				if (state_it == context.back().local_state_vals.end()) {
					state_it = context.back().local_state_vals.insert({this->state_ids[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				state_network->activate(obs_it->second,
										state_it->second);
			} else {
				map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_ids[n_index]);
				if (state_it != context.back().input_state_vals.end()) {
					StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
					state_network->activate(obs_it->second,
											state_it->second);
				}
			}
		}
	}

	for (int n_index = 0; n_index < (int)this->score_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->score_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->score_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->score_state_scope_contexts[n_index][c_index] != context[context.size()-this->score_state_scope_contexts[n_index].size()+c_index].scope_id
						|| this->score_state_node_contexts[n_index][c_index] != context[context.size()-this->score_state_scope_contexts[n_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->score_state_obs_indexes[n_index]);
			if (obs_it != history->obs_snapshots.end()) {
				map<State*, StateStatus>::iterator state_it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.find(this->score_state_defs[n_index]);
				if (state_it == context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.end()) {
					state_it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.insert({this->score_state_defs[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->score_state_defs[n_index]->networks[this->score_state_network_indexes[n_index]];
				state_network->activate(obs_it->second,
										state_it->second);
			}
		}
	}

	context.pop_back();

	context.back().node_id = -1;
}
