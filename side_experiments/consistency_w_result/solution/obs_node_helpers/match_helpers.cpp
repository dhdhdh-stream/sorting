#include "obs_node.h"

#include <algorithm>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

bool match_comp(Match first, Match second) {
	if (first.average_distance > second.average_distance) {
		return true;
	} else {
		return false;
	}
}

void ObsNode::gather_match_datapoints(ObsNodeHistory* history,
									  ScopeHistory* scope_history) {
	if (!this->is_init) {
		for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
				it != scope_history->node_histories.end(); it++) {
			if (it->second->index < history->index
					&& it->second->node->type == NODE_TYPE_OBS) {
				int match_index = -1;
				for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
					if (this->matches[m_index].node_context[0] == it->first) {
						match_index = m_index;
						break;
					}
				}
				if (match_index == -1) {
					Match new_match;
					new_match.parent = this;
					new_match.scope_context = {this->parent};
					new_match.node_context = {it->first};
					new_match.is_init = false;
					this->matches.push_back(new_match);
					match_index = (int)this->matches.size()-1;
				}

				ObsNodeHistory* early_history = (ObsNodeHistory*)it->second;
				this->matches[match_index].datapoints.push_back(
					{early_history->obs_history[0], history->obs_history[0]});
				this->matches[match_index].distances.push_back(
					history->num_true_actions - early_history->num_true_actions);
			}
		}
	} else {
		for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
				it != scope_history->node_histories.end(); it++) {
			if (it->second->node->is_init
					&& it->second->node->type == NODE_TYPE_OBS
					&& it->second->index < history->index) {
				bool has_match = false;
				for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
					if (this->matches[m_index].node_context[0] == it->first) {
						has_match = true;
						break;
					}
				}
				if (!has_match) {
					Match new_match;
					new_match.parent = this;
					new_match.scope_context = {this->parent};
					new_match.node_context = {it->first};
					new_match.is_init = false;
					this->matches.push_back(new_match);
				}
			}
		}

		for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
			map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories
				.find(this->matches[m_index].node_context[0]);
			if (it != scope_history->node_histories.end()) {
				ObsNodeHistory* early_history = (ObsNodeHistory*)it->second;

				this->matches[m_index].datapoints.push_back(
					{early_history->obs_history[0], history->obs_history[0]});
				this->matches[m_index].distances.push_back(
					{history->num_true_actions - early_history->num_true_actions});
			}
		}
	}
}

void ObsNode::update_matches() {
	for (int m_index = (int)this->matches.size()-1; m_index >= 0; m_index--) {
		bool is_still_needed;
		this->matches[m_index].update(is_still_needed);
		if (!is_still_needed) {
			this->matches.erase(this->matches.begin() + m_index);
		}
	}

	sort(this->matches.begin(), this->matches.end(), match_comp);
}
