#include "action_node.h"

using namespace std;

void ActionNode::activate(vector<double>& flat_vals,
						  vector<ContextLayer>& context,
						  vector<AbstractNodeHistory*>& node_histories) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	node_histories.push_back(history);

	history->obs_snapshot = flat_vals[0];
	flat_vals.erase(flat_vals.begin());

	history->state_snapshots = vector<double>(this->local_state_ids.size());
	for (int n_index = 0; n_index < (int)this->local_state_ids.size(); n_index++) {
		map<int, StateStatus>::iterator it = context.back().state_vals.find(this->local_state_ids[n_index]);
		if (it == context.back().state_vals.end()) {
			it = context.back().state_vals.insert({this->local_state_ids[n_index], StateStatus()}).first;
		}
		StateNetwork* state_network = this->states[n_index]->networks[this->network_indexes[n_index]];
		if (this->obs_ids[n_index] == -1) {
			state_network->activate(history->obs_snapshot,
									it->second);
		} else {
			state_network->activate(history->state_snapshots[this->obs_ids[n_index]],
									it->second);
		}
		history->state_snapshots[n_index] = it->second.val;
		/**
		 * - don't worry about normalizing state_snapshots
		 *   - values were just normalized going in, so output should be consistent
		 */
	}

	for (int n_index = 0; n_index < (int)this->score_states.size(); n_index++) {
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
			map<ScoreState*, StateStatus>::iterator it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.find(this->score_states[n_index]);
			if (it == context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.end()) {
				it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.insert({this->score_states[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->score_states[n_index]->networks[this->score_network_indexes[n_index]];
			if (this->score_obs_ids[n_index] == -1) {
				state_network->activate(history->obs_snapshot,
										it->second);
			} else {
				state_network->activate(history->state_snapshots[this->score_obs_ids[n_index]],
										it->second);
			}
		}
	}
}
