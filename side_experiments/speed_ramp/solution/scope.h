#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <vector>

#include "input.h"

class AbstractExperiment;
class AbstractNode;
class AbstractNodeHistory;
class Factor;
class SignalExperiment;
class SignalInstance;
class Solution;
class SolutionWrapper;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Factor*> factors;

	std::vector<Scope*> child_scopes;

	Scope();
	~Scope();

	void signal_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);

	void invalidate_factor(ScopeHistory* scope_history,
						   int f_index);

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void copy_from(Scope* original,
				   Solution* parent_solution);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

/**
 * TODO: calculate and assign signals on exit
 */
class ScopeHistory {
public:
	Scope* scope;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<bool> factor_initialized;
	std::vector<double> factor_values;

	int num_actions_snapshot;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */