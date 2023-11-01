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
	/**
	 * - recursion possible even when exploring with only child scopes
	 *   - should be less likely though so don't specifically check against it
	 *     - (and don't have to rely on recursion with OuterExperiment)
	 */

	std::map<AbstractExperiment*, int> experiments_seen;
	/**
	 * - keep track of counts for branch
	 */
	std::vector<AbstractExperiment*> experiments_seen_order;

	AbstractExperiment* selected_experiment;

	int branch_experiment_count;
	AbstractExperimentHistory* experiment_history;
	/**
	 * - instance for branch, overall for passthrough
	 */

	RunHelper() {
		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		this->selected_experiment = NULL;
		this->experiment_history = NULL;
	}
};

#endif /* RUN_HELPER_H */