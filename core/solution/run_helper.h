#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;

class RunHelper {
public:
	int curr_depth;
	int max_depth;

	bool exceeded_limit;

	int num_decisions;
	int num_actions;

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