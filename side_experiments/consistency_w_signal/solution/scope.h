// TODO: try to understand why signals can be false
// TODO: try to classify when a signal is maxed
// - which means can also classify when a signal can be chased

// - only useful signals are ones that can be chased?
//   - anything else mainly just reinforces existing?

// - if signal appears multiple times, track global
//   - then see if global improves as score improves

// - likely can always find scenarios where signal improves but true is worse
//   - so should give some number of tries?

// - also possible for signal to truly improve true, but hurt in some way
//   - such that correlation isn't perfect

// - still use all signals
//   - so best are paths that are good on all signals

// - if signal does not improve, or if signal improves but true doesn't, then remove
//   - so only keep if signal improves while results improve

// - maybe also divide signals into established, and testing
//   - and can remove established after a while
//   - established are what can be used in other signals inwards

/**
 * - OK if scope used in or moved into new context
 *   - won't be recognized as existing so signal will be ignored
 *     - then adjusts later
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

class AbstractNode;
class AbstractNodeHistory;
class Factor;
class Network;
class Problem;
class Solution;
class SolutionWrapper;

const int SIGNAL_STATUS_INIT = 0;
const int SIGNAL_STATUS_TEST = 1;
const int SIGNAL_STATUS_ESTABLISHED = 2;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;
	/**
	 * TODO: can hardcode link to starting node
	 */

	std::vector<Scope*> child_scopes;

	/**
	 * - train on explore
	 *   - OK if bad average due to experiment's train existing
	 * 
	 * - don't worry about pre vs. post as some scopes will be evaluated fully after explore
	 */
	int signal_status;
	Network* consistency_network;
	Network* signal_network;

	std::vector<std::vector<double>> existing_pre_obs;
	std::vector<std::vector<double>> existing_post_obs;
	int existing_index;
	/**
	 * - if not enough samples from new, simply reuse previous
	 */

	std::vector<std::vector<double>> explore_pre_obs;
	std::vector<std::vector<double>> explore_post_obs;
	std::vector<double> explore_target_vals;

	std::vector<double> signal_history;

	Scope();
	~Scope();

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	void update_signals(SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<double> pre_obs;
	std::vector<double> post_obs;

	bool has_explore;

	bool signal_initialized;
	double signal_val;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */