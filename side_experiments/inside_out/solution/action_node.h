#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"

class Problem;
class ScopeHistory;
class SolutionWrapper;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	int action;

	int next_node_id;
	AbstractNode* next_node;

	double average_hits_per_run;
	double average_instances_per_run;
	double average_score;

	int last_updated_run_index;
	double sum_score;
	int sum_hits;
	int sum_instances;

	double new_scope_average_hits_per_run;
	double new_scope_average_score;

	double new_scope_sum_score;
	int new_scope_sum_count;

	ActionNode();
	~ActionNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	void clean();
	void measure_update(int total_count);

	void new_scope_clean();
	void new_scope_measure_update(int total_count);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	ActionNodeHistory(ActionNode* node);
	ActionNodeHistory(ActionNodeHistory* original);
};

#endif /* ACTION_NODE_H */