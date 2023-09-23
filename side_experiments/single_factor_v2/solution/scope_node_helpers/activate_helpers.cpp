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
			map<int, double>::iterator it = context.back().state_vals.find(this->input_outer_ids[i_index]);
			if (it != context.back().state_vals.end()) {
				inner_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = it->second;
			}
		} else {
			inner_state_vals[this->input_inner_layers[i_index]][this->input_inner_ids[i_index]] = this->input_init_vals[i_index];
		}
	}

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = inner_state_vals[0];
	inner_state_vals.erase(inner_state_vals.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	// currently, starting_node_ids.size() == inner_state_vals.size()+1

	inner_scope->activate(starting_node_ids_copy,
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

	for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
		map<int, double>::iterator it = context.back().find(this->output_inner_ids[o_index]);
		if (it != context.back().end()) {
			context[context.size()-2].state_vals[this->output_outer_ids[o_index]] = it->second;
		}
	}

	context.pop_back();

	context.back().node_id = -1;
}

void ScopeNode::halfway_activate(vector<int>& starting_node_ids,
								 vector<map<int, double>>& starting_state_vals,
								 vector<double>& flat_vals,
								 vector<ContextLayer>& context,
								 int& inner_exit_depth,
								 int& inner_exit_node_id,
								 RunHelper& run_helper,
								 ScopeNodeHistory* history) {
	history->is_halfway = true;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	context.back().node_id = this->id;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope_id;
	context.back().node_id = -1;

	context.back().state_vals = starting_state_vals[0];
	starting_state_vals.erase(starting_state_vals.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	// currently, starting_node_ids.size() == starting_state_vals.size()+1

	inner_scope->activate(starting_node_ids,
						  starting_state_vals,
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

	for (int o_index = 0; o_index < (int)this->output_inner_ids.size(); o_index++) {
		map<int, double>::iterator it = context.back().find(this->output_inner_ids[o_index]);
		if (it != context.back().end()) {
			context[context.size()-2].state_vals[this->output_outer_ids[o_index]] = it->second;
		}
	}

	context.pop_back();

	context.back().node_id = -1;
}
