#include "obs_node.h"

using namespace std;

void ObsNode::measure_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   RunHelper& run_helper,
							   ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		double curr_variance = (solution->obs_average_val[o_index] - obs[o_index])
			* (solution->obs_average_val[o_index] - obs[o_index]);

		solution->obs_average_val[o_index] = 0.0001 * obs[o_index] + 0.9999 * solution->obs_average_val[o_index];
		solution->obs_variance[o_index] = 0.0001 * curr_variance + 0.9999 * solution->obs_variance[o_index];
	}
	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	for (int k_index = 0; k_index < (int)this->keypoints.size(); k_index++) {
		if (this->keypoints[k_index] != NULL) {
			this->keypoints[k_index]->measure_activate(run_helper,
													   scope_history);
		}
	}

	curr_node = this->next_node;
}
