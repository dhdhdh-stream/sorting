#include "obs_node.h"

using namespace std;

void ObsNode::experiment_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		if (this->keypoints[k_index] != NULL) {
			this->keypoints[k_index]->activate(run_helper,
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
