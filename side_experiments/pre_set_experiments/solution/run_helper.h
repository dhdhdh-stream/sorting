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

	int num_analyze;
	int num_actions;

	std::vector<AbstractNode*> experiments_seen;

	AbstractNode* experiment_node;
	AbstractExperimentHistory* experiment_history;

	#if defined(MDEBUG) && MDEBUG
	unsigned long starting_run_seed;
	unsigned long curr_run_seed;
	#endif /* MDEBUG */

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */