#include "obs_node.h"

#include <iostream>

#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "problem.h"

using namespace std;

void ObsNode::measure_match_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	history->num_matches = scope_history->num_matches;
	/**
	 * - set early to include local matches
	 */
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	if (this->is_fixed_point) {
		if (abs(obs[0] - this->average) < MIN_STANDARD_DEVIATION) {
			scope_history->num_matches++;
		}
	}
	/**
	 * - check every match to check on all earlier nodes
	 */
	for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
		map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories
			.find(this->matches[m_index].node_context[0]);
		if (it != scope_history->node_histories.end()) {
			ObsNodeHistory* early_history = (ObsNodeHistory*)it->second;

			double predicted_score = obs[0] * this->matches[m_index].weight + this->matches[m_index].constant;
			if (abs(early_history->obs_history[0] - predicted_score) < MIN_STANDARD_DEVIATION) {
				scope_history->num_matches++;
			}
		}
	}


	curr_node = this->next_node;
}
