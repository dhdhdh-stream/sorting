#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <random>
#include <set>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractNode;

class RunHelper {
public:
	bool can_random;
	std::uniform_int_distribution<int> random_distribution;

	int num_analyze;
	int num_actions;

	std::vector<AbstractExperiment*> experiments_seen_order;
	AbstractExperimentHistory* experiment_history;

	#if defined(MDEBUG) && MDEBUG
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	RunHelper();
	~RunHelper();

	bool is_random();
};

#endif /* RUN_HELPER_H */