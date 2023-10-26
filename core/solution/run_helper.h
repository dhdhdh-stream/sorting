#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <set>
#include <vector>

class BranchExperiment;
class BranchExperimentHistory;
class ScopeHistory;

class RunHelper {
public:
	int phase;

	int curr_depth;
	int max_depth;
	bool exceeded_depth;

	BranchExperiment* selected_branch_experiment;
	int selected_branch_experiment_count;
	BranchExperimentHistory* branch_experiment_history;
	/**
	 * - also use to track if already experimented (i.e., check if not NULL)
	 */
	ScopeHistory* branch_experiment_scope_history;

	std::vector<BranchExperiment*> experiments_seen_order;
	std::map<BranchExperiment*, int> experiments_seen_counts;

	RunHelper() {
		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		this->selected_branch_experiment = NULL;
		this->branch_experiment_history = NULL;
		this->branch_experiment_scope_history = NULL;
	}
};

#endif /* RUN_HELPER_H */