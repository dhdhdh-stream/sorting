#ifndef RUN_HELPER_H
#define RUN_HELPER_H

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

	std::set<AbstractNode*> branch_node_ancestors;
	/**
	 * - to help enable recursion
	 */
	std::set<ScopeNode*> scope_node_ancestors;
	/**
	 * - to prevent unbounded recursion
	 */

	int selected_count;
	std::pair<AbstractNode*,bool> selected_node;

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