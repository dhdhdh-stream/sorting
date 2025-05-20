#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "factor.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ObsNode::experiment_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	if (run_helper.check_match) {
		if (this->is_fixed_point) {
			double factor = abs(obs[0] - this->average) / this->standard_deviation;
			run_helper.match_factors.push_back(factor);
		}
		/**
		 * - check every match to check on all earlier nodes
		 */
		for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
			this->matches[m_index].eval(obs,
										run_helper,
										scope_history);
		}
	}

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->activate(
			this,
			false,
			curr_node,
			problem,
			run_helper,
			scope_history);
	}
}
