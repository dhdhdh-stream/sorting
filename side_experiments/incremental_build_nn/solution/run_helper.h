#ifndef RUN_HELPER_H
#define RUN_HELPER_H

class RunHelper {
public:
	int curr_depth;
	int max_depth;

	bool exceeded_limit;

	std::vector<AbstractExperiment*> experiments_seen_order;

	AbstractExperimentHistory* experiment_history;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

};

#endif /* RUN_HELPER_H */