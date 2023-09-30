#include "action_node.h"

using namespace std;

void ActionNode::activate(vector<double>& flat_vals,
						  vector<ContextLayer>& context,
						  vector<AbstractNodeHistory*>& node_histories) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	node_histories.push_back(history);

	history->obs_snapshot = flat_vals[0];
	flat_vals.erase(flat_vals.begin());

	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		if (this->state_is_local[n_index]) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_ids[n_index]);
			if (it == context.back().local_state_vals.end()) {
				it = context.back().local_state_vals.insert({this->state_ids[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
			state_network->activate(history->obs_snapshot,
									it->second);
		} else {
			map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_ids[n_index]);
			if (it != context.back().input_state_vals.end()) {
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				state_network->activate(history->obs_snapshot,
										it->second);
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
			map<State*, StateStatus>::iterator it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.find(this->score_state_defs[n_index]);
			if (it == context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.end()) {
				it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.insert({this->score_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->score_state_defs[n_index]->networks[this->score_state_network_indexes[n_index]];
			StateStatus score_state_impact;
			state_network->activate_score(history->obs_snapshot,
										  it->second,
										  score_state_impact);
			history->score_state_indexes.push_back(n_index);
			history->score_state_impacts.push_back(score_state_impact);
		}
	}

	for (int n_index = 0; n_index < (int)this->experiment_hook_score_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_hook_score_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_hook_score_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_hook_score_state_scope_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index].scope_id
						|| this->experiment_hook_score_state_node_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<State*, StateStatus>::iterator it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()]
				.experiment_score_state_vals.find(this->experiment_hook_score_state_defs[n_index]);
			if (it == context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals.end()) {
				it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals
					.insert({this->experiment_hook_score_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_hook_score_state_defs[n_index]->networks[this->experiment_hook_score_state_network_indexes[n_index]];
			state_network->activate(history->obs_snapshot,
									it->second);
		}
	}

	for (int h_index = 0; h_index < (int)this->test_hook_histories.size(); h_index++) {
		bool matches_context = true;
		if (this->test_hook_scope_contexts[h_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->test_hook_scope_contexts[h_index].size()-1; c_index++) {
				if (this->test_hook_scope_contexts[h_index][c_index] != context[context.size()-this->test_hook_scope_contexts[h_index].size()+c_index].scope_id
						|| this->test_hook_node_contexts[h_index][c_index] != context[context.size()-this->test_hook_scope_contexts[h_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			context[context.size()-this->test_hook_scope_contexts[h_index].size()]
				.scope_history->test_obs_indexes.push_back(this->test_hook_indexes[h_index]);
			context[context.size()-this->test_hook_scope_contexts[h_index].size()]
				.scope_history->test_obs_vals.push_back(history->obs_snapshot);
		}
	}
}

void ActionNode::experiment_back_activate(vector<int>& scope_context,
										  vector<int>& node_context,
										  map<State*, StateStatus>& experiment_score_state_vals,
										  vector<int>& test_obs_indexes,
										  vector<double>& test_obs_vals,
										  ActionNodeHistory* history) {
	for (int n_index = 0; n_index < (int)this->experiment_hook_score_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_hook_score_state_scope_contexts[n_index].size() > scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_hook_score_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_hook_score_state_scope_contexts[n_index][c_index] != scope_context[scope_context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index]
						|| this->experiment_hook_score_state_node_contexts[n_index][c_index] != node_context[scope_context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<State*, StateStatus>::iterator it = experiment_score_state_vals.find(this->experiment_hook_score_state_defs[n_index]);
			if (it == experiment_score_state_vals.end()) {
				it = experiment_score_state_vals.insert({this->experiment_hook_score_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_hook_score_state_defs[n_index]->networks[this->experiment_hook_score_state_network_indexes[n_index]];
			state_network->activate(history->obs_snapshot,
									it->second);
		}
	}

	for (int h_index = 0; h_index < (int)this->test_hook_histories.size(); h_index++) {
		bool matches_context = true;
		if (this->test_hook_scope_contexts[h_index].size() > scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->test_hook_scope_contexts[h_index].size()-1; c_index++) {
				if (this->test_hook_scope_contexts[h_index][c_index] != scope_context[scope_context.size()-this->test_hook_scope_contexts[h_index].size()+c_index]
						|| this->test_hook_node_contexts[h_index][c_index] != node_context[scope_context.size()-this->test_hook_scope_contexts[h_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			test_obs_indexes.push_back(this->test_hook_indexes[h_index]);
			test_obs_vals.push_back(history->obs_snapshot);
		}
	}
}
