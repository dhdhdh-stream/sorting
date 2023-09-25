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
			state_network->activate(history->obs_snapshot,
									it->second);
		}
	}
}
