#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <set>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;

class RunHelper {
public:
	int num_actions;

	int curr_depth;
	int max_depth;

	bool exceeded_limit;
	/**
	 * - recursion possible even when exploring with only child scopes
	 *   - should be less likely though so don't specifically check against it
	 *     - (and don't have to rely on recursion with OuterExperiment)
	 */

	bool has_exited;

	std::set<AbstractExperiment*> experiments_seen;
	std::vector<AbstractExperiment*> experiments_seen_order;

	AbstractExperimentHistory* experiment_history;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	RunHelper() {
		this->num_actions = 0;

		this->curr_depth = 0;
		this->max_depth = 0;

		this->exceeded_limit = false;

		this->has_exited = false;

		this->experiment_history = NULL;
	}
};

#endif /* RUN_HELPER_H */