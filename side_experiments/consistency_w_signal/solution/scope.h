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

const int SIGNAL_STATUS_INIT = 0;
const int SIGNAL_STATUS_VALID = 1;
const int SIGNAL_STATUS_FAIL = 2;

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
	 * - simply train both pre_network and post_network on explore
	 *   - doesn't really matter what pre_network trained on due to experiment's train existing
	 */
	int signal_status;
	Network* consistency_network;
	Network* pre_network;
	Network* post_network;

	std::vector<std::vector<double>> existing_pre_obs;
	std::vector<std::vector<double>> existing_post_obs;
	int existing_index;
	/**
	 * - if not enough samples from new, simply reuse previous
	 */

	std::vector<std::vector<double>> explore_pre_obs;
	std::vector<std::vector<double>> explore_post_obs;
	std::vector<double> explore_target_vals;

	// temp
	std::vector<double> explore_signals;
	std::vector<double> explore_true;

	Scope();
	~Scope();

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	void update_signals();

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);

	// temp
	void measure_signal_pcc();
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