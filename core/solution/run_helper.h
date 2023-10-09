#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <vector>

class BranchExperiment;
class BranchExperimentHistory;
class ScopeHistory;

class RunHelper {
public:
	int phase;

	int node_index;

	int curr_depth;	// need to track separate from context as context resets for experiments
	int max_depth;
	bool exceeded_depth;

	/**
	 * - don't need to track running tally of predicted score
	 *   - instead, on experiment, calculate from partial and resolved score_state_vals
	 */

	std::vector<ScopeHistory*> scope_histories;

	std::vector<BranchExperiment*> experiments_seen_order;
	std::map<BranchExperiment*, int> experiments_seen_counts;
	/**
	 * - also use to track start for explore
	 */

	BranchExperiment* selected_branch_experiment;
	int selected_branch_experiment_count;
	BranchExperimentHistory* branch_experiment_history;
	/**
	 * - also use to track if already experimented (i.e., check if not NULL)
	 */

	RunHelper() {
		this->node_index = 0;

		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		this->selected_branch_experiment = NULL;
		this->branch_experiment_history = NULL;
	}
};

#endif /* RUN_HELPER_H */