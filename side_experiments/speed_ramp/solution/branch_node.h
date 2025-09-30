#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "input.h"

class Problem;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	double constant;
	std::vector<Input> inputs;
	std::vector<double> input_averages;
	std::vector<double> input_standard_deviations;
	std::vector<double> weights;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	std::vector<int> impacted_factors;

	int original_num_experiments;
	int branch_num_experiments;

	BranchNode();
	~BranchNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	int num_actions_snapshot;

	BranchNodeHistory(BranchNode* node);
	BranchNodeHistory(BranchNodeHistory* original);
};

#endif /* BRANCH_NODE_H */