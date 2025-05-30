#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
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
		bool has_match = false;
		for (int m_index = 0; m_index < (int)this->matches.size(); m_index++) {
			map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories
				.find(this->matches[m_index].node_context[0]);
			if (it != scope_history->node_histories.end()) {
				ObsNodeHistory* early_history = (ObsNodeHistory*)it->second;

				double predicted_score = obs[0] * this->matches[m_index].weight + this->matches[m_index].constant;
				double factor = abs(predicted_score - early_history->obs_history[0]) / this->matches[m_index].standard_deviation;
				run_helper.match_factors.push_back(factor);

				has_match = true;
				break;
			}
		}
		if (!has_match && this->check_consistency) {
			double factor = abs(obs[0] - this->average) / this->standard_deviation;
			run_helper.match_factors.push_back(factor);
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
