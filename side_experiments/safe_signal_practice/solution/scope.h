#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class AbstractExperiment;
class AbstractNode;
class AbstractNodeHistory;
class Network;
class Solution;
class SolutionWrapper;

class ScopeHistory;
class Scope {
public:
	bool is_outer;
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;
	/**
	 * TODO: can hardcode link to starting node
	 */

	std::vector<Scope*> child_scopes;

	Network* simple_existing_signal;
	std::vector<std::vector<double>> existing_obs_histories;
	std::vector<double> existing_target_val_histories;
	int existing_history_index;
	/**
	 * - have existing signal to reduce noise
	 */

	Network* simple_explore_signal;
	std::vector<std::vector<double>> explore_obs_histories;
	std::vector<double> explore_target_val_histories;
	int explore_history_index;

	Scope();
	~Scope();

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);
	void random_activate(std::vector<AbstractNode*>& path);

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

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */