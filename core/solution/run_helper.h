/**
 * - experimenting multiple times during a run is risky
 *   - bad explore may lead to irrecoverable/irrepresentative state
 *   - instead, explore once, but use eval to magnify impact/reduce variance
 *     - hopefully better results with fewer samples
 *       - (and even fewer overall tries)
 * 
 * - for humans:
 *   - world modeling to avoid permanent damage
 *     - though negatively impacts explore
 *   - large number of reward signals during lifetime
 *     - so life can be easily broken up into many separate tries
 *   - if suffer permanent damage, explore/decision making will permanently adjust accordingly
 */

#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <set>
#include <utility>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;
class AbstractNode;
class AbstractScope;
class InfoScope;
class Problem;
class Scope;
class ScopeHistory;
class ScopeNode;
class Solution;

class RunHelper {
public:
	bool exceeded_limit;

	int num_decisions;
	int num_actions;

	std::set<AbstractNode*> branch_node_ancestors;
	/**
	 * - to help enable recursion
	 */
	std::set<ScopeNode*> scope_node_ancestors;
	/**
	 * - to prevent unbounded recursion
	 */

	/**
	 * - choose randomly rather than proportional to use
	 *   - proportional biases towards lower layers, but upper layers may be just as impactful
	 */
	std::map<std::pair<AbstractNode*,bool>, int> scope_nodes_seen;
	std::map<AbstractNode*, int> info_scope_nodes_seen;

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