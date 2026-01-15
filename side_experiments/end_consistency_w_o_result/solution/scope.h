/**
 * - train signals on solution history
 *   - don't train on explore
 *     - anything actually good will become part of solution anyways
 *     - crazily bad samples unlikely to lead to helpful signals
 *     - slightly different, but not worse samples do not lead to consistency
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

	std::vector<std::vector<double>> existing_pre_obs;
	std::vector<std::vector<double>> existing_post_obs;

	std::vector<std::vector<double>> explore_pre_obs;
	std::vector<std::vector<double>> explore_post_obs;

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

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<double> pre_obs;
	std::vector<double> post_obs;

	ScopeHistory(Scope* scope);
	~ScopeHistory();

	ScopeHistory* copy_signal();
};

#endif /* SCOPE_H */