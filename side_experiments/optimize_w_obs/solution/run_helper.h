#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <set>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractNode;
class ScopeNode;

class RunHelper {
public:
	double result;

	bool exceeded_limit;

	int num_analyze;
	int num_actions;

	std::map<std::pair<AbstractNode*,bool>, int> nodes_seen;

	std::vector<AbstractExperiment*> experiments_seen_order;
	std::vector<AbstractExperimentHistory*> experiment_histories;

	#if defined(MDEBUG) && MDEBUG
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */