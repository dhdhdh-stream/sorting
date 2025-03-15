#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <set>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractNode;
class MultiPassThroughExperiment;
class MultiPassThroughExperimentHistory;

class RunHelper {
public:
	int num_analyze;
	int num_actions;

	std::vector<AbstractExperiment*> experiments_seen_order;
	AbstractExperimentHistory* experiment_history;

	bool has_explore;

	std::map<MultiPassThroughExperiment*, MultiPassThroughExperimentHistory*> multi_experiment_histories;

	#if defined(MDEBUG) && MDEBUG
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */