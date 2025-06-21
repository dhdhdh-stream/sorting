#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "abstract_node.h"

class Problem;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	Scope* scope;

	int next_node_id;
	AbstractNode* next_node;

	double average_hits_per_run;
	double average_score;

	int last_updated_run_index;
	double sum_score;
	int sum_count;

	double new_scope_average_hits_per_run;
	double new_scope_average_score;

	double new_scope_sum_score;
	int new_scope_sum_count;

	ScopeNode();
	~ScopeNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);
	void exit_step(SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);

	void explore_step(std::vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper);
	void explore_exit_step(SolutionWrapper* wrapper);

	void replace_scope(Scope* original_scope,
					   Scope* new_scope);

	void clean();
	void measure_update();

	void new_scope_clean();
	void new_scope_measure_update(int total_count);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */