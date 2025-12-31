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
class Tunnel;

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

	std::vector<Tunnel*> tunnels;

	std::vector<std::vector<ScopeHistory*>> explore_stack_traces;
	std::vector<double> explore_target_val_histories;
	/**
	 * - don't save/load
	 *   - can regather on first iters
	 */

	Scope();
	~Scope();

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<double> obs_history;

	std::vector<bool> tunnel_is_init;
	std::vector<double> tunnel_vals;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original,
				 Solution* parent_solution);
	~ScopeHistory();

	ScopeHistory* copy_obs_history();
};

#endif /* SCOPE_H */