#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>

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

	ActionNode();
	ActionNode(ActionNode* original);

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

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