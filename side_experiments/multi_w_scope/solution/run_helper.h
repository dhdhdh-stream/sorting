#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <set>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractNode;

class RunHelper {
public:
	int num_actions;

	int num_original_actions;
	int num_experiment_instances;

	std::map<AbstractExperiment*, AbstractExperimentHistory*> experiment_histories;

	bool has_explore;

	#if defined(MDEBUG) && MDEBUG
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */