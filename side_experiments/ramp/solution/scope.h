#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
#include <set>
#include <vector>

#include "run_helper.h"

class AbstractNode;
class AbstractNodeHistory;
class NewScopeExperiment;
class Problem;
class Solution;

class ScopeHistory;
class Scope {
public:
	int id;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Scope*> child_scopes;

	bool exceeded;
	bool generalized;

	Scope* new_scope;
	std::vector<NewScopeExperiment*> test_experiments;
	int generalize_iter;
	std::vector<NewScopeExperiment*> successful_experiments;

	Scope();
	~Scope();

	void activate(Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);

	void experiment_activate(Problem* problem,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

	void check_new_scope(std::set<Scope*>& updated_scopes);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_factor(Scope* scope,
						int original_node_id,
						int original_factor_index,
						int new_node_id,
						int new_factor_index);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);
	void replace_scope(Scope* original_scope,
					   Scope* new_scope,
					   int new_scope_node_id);

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