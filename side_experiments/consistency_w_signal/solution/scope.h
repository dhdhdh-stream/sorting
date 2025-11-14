/**
 * - update consistency between explore and experiment
 * - update signal after experiment update
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

	Network* consistency_network;

	std::vector<std::vector<double>> consistency_existing_pre_obs;
	std::vector<std::vector<double>> consistency_existing_post_obs;
	int consistency_existing_index;

	std::vector<std::vector<double>> consistency_explore_pre_obs;
	std::vector<std::vector<double>> consistency_explore_post_obs;
	int consistency_explore_index;

	Network* pre_network;
	Network* post_network;

	std::vector<std::vector<double>> signal_pre_obs;
	std::vector<std::vector<double>> signal_post_obs;
	std::vector<double> signal_target_vals;
	int signal_index;

	Scope();
	~Scope();

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	void update_consistency();
	void update_signals();

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

	bool signal_initialized;
	double pre_val;
	double post_val;

	ScopeHistory(Scope* scope);
	~ScopeHistory();

	ScopeHistory* copy_signal();
};

#endif /* SCOPE_H */