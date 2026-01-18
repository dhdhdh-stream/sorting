#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"

class Network;
class Problem;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	Network* network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	double original_impact;
	double branch_impact;

	double original_update_sum_vals;
	double original_update_counts;
	double branch_update_sum_vals;
	double branch_update_counts;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

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

	#if defined(MDEBUG) && MDEBUG
	void verify_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);
	void clear_verify();
	#endif /* MDEBUG */

	void attribute_update();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(BranchNode* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */