#include "obs_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "confusion.h"
#include "factor.h"
#include "problem.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void ObsNode::experiment_step(vector<double>& obs,
							  int& action,
							  bool& is_next,
							  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	history->obs_history = obs;

	if (wrapper->measure_match) {
		map<ObsNode*, ObsData>::iterator it = wrapper->experiment_history
			->experiment->existing_obs_data.find(this);
		if (it != wrapper->experiment_history->experiment->existing_obs_data.end()) {
			for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
				double t_score = (obs[o_index] - it->second.averages[o_index])
					/ it->second.standard_deviations[o_index];
				wrapper->t_scores.push_back(t_score);
			}
		}
	}

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	wrapper->node_context.back() = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->check_activate(
			this,
			false,
			wrapper);
	} else if (this->confusion != NULL) {
		this->confusion->check_activate(wrapper);
	}
}
