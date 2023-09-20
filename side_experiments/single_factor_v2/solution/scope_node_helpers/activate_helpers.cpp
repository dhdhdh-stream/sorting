#include "scope_node.h"

using namespace std;

void ScopeNode::activate(vector<double>& flat_vals,
						 vector<ContextLayer>& context,
						 int& inner_exit_depth,
						 int& inner_exit_node_id,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
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

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = &(inner_state_vals[0]);

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	vector<vector<double>*> inner_state_vals_copy(this->starting_node_ids.size()-1);
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		inner_state_vals_copy[l_index] = &(inner_state_vals[1+l_index]);
	}

	// currently, starting_node_ids.size() == inner_state_vals_copy.size()+1

	inner_scope->activate(starting_node_ids_copy,
						  inner_state_vals_copy,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	for (map<int, ScoreStateStatus>::iterator it = context.back().score_state_vals.begin();
			it != context.back().score_state_vals.end(); it++) {
		predicted_score -= it->second.predicted_score_mod;
		predicted_score += it->second.val*solution->score_states[it->first]->scale->weight;
		map<int, double> overall_it = run_helper.score_state_vals.find(it->first);
		if (overall_it != run_helper.score_state_vals.end()) {
			overall_it->second += it->second.val;
		} else {
			run_helper.score_state_vals[it->first] = it->second.val;
		}
	}

	context.pop_back();

	context.back().node_id = -1;

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

void ScopeNode::halfway_activate(vector<int>& starting_node_ids,
								 vector<map<int, double>*>& starting_state_vals,
								 vector<double>& flat_vals,
								 vector<ContextLayer>& context,
								 int& inner_exit_depth,
								 int& inner_exit_node_id,
								 RunHelper& run_helper,
								 ScopeNodeHistory* history) {
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

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = starting_state_vals[0];

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<vector<double>*> inner_state_vals(starting_state_vals.size()-1);
	for (int l_index = 0; l_index < (int)starting_state_vals.size()-1; l_index++) {
		inner_state_vals[l_index] = starting_state_vals[1+l_index];
	}

	// currently, starting_node_ids.size() == inner_state_vals.size()+1

	inner_scope->activate(starting_node_ids,
						  inner_state_vals,
						  flat_vals,
						  context,
						  inner_exit_depth,
						  inner_exit_node_id,
						  run_helper,
						  inner_scope_history);

	for (map<int, ScoreStateStatus>::iterator it = context.back().score_state_vals.begin();
			it != context.back().score_state_vals.end(); it++) {
		predicted_score -= it->second.predicted_score_mod;
		predicted_score += it->second.val*solution->score_states[it->first]->scale->weight;
		map<int, double> overall_it = run_helper.score_state_vals.find(it->first);
		if (overall_it != run_helper.score_state_vals.end()) {
			overall_it->second += it->second.val;
		} else {
			run_helper.score_state_vals[it->first] = it->second.val;
		}
	}

	context.pop_back();

	context.back().node_id = -1;

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
