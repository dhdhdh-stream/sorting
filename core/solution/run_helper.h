#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;

class RunHelper {
public:
	bool can_restart;
	bool should_restart;

	int curr_depth;
	int max_depth;

	int throw_id;

	bool exceeded_limit;

	/**
	 * - for starting experiments
	 */
	std::vector<AbstractExperiment*> experiments_seen_order;

	std::vector<AbstractExperimentHistory*> experiment_histories;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */