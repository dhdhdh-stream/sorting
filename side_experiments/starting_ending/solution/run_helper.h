#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <set>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;

class RunHelper {
public:
	int curr_depth;
	int max_depth;
	bool exceeded_depth;
	/**
	 * - recursion possible even when exploring with only child scopes
	 *   - should be less likely though so don't specifically check against it
	 *     - (and don't have to rely on recursion with OuterExperiment)
	 */

	std::set<AbstractExperiment*> experiments_seen;
	std::vector<AbstractExperiment*> experiments_seen_order;

	void* selected_experiment;
	AbstractExperimentHistory* experiment_history;

	RunHelper() {
		this->curr_depth = 0;
		this->max_depth = 0;
		this->exceeded_depth = false;

		this->selected_experiment = NULL;
		this->experiment_history = NULL;
	}
};

#endif /* RUN_HELPER_H */