#ifndef BRANCH_END_NODE_H
#define BRANCH_END_NODE_H

#include <fstream>
#include <utility>
#include <vector>

#include "abstract_node.h"

class BranchNode;
class Network;
class Problem;
class Solution;
class SolutionWrapper;

class BranchEndNodeHistory;
class BranchEndNode : public AbstractNode {
public:
	int branch_start_node_id;
	BranchNode* branch_start_node;

	int next_node_id;
	AbstractNode* next_node;

	BranchEndNode();
	~BranchEndNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void result_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class BranchEndNodeHistory : public AbstractNodeHistory {
public:
	BranchEndNodeHistory(BranchEndNode* node);
};

#endif /* BRANCH_END_NODE_H */