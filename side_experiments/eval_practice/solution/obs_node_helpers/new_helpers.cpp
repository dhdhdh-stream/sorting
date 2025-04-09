#include "obs_node.h"

#include <iostream>

#include "problem.h"
#include "scope.h"

using namespace std;

void ObsNode::new_activate(Problem* problem,
						   ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;
}
