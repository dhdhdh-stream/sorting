#include "obs_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"

using namespace std;

void ObsNode::commit_activate(Problem* problem,
							  RunHelper& run_helper,
							  ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());
}
