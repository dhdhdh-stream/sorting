#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include "abstract_node.h"
#include "run_helper.h"

#include <utility>
#include <vector>

class Problem;
class ScopeHistory;
class Solution;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	double average_val;
	std::vector<std::pair<int,int>> factor_ids;
	std::vector<double> factor_weights;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	BranchNode();
	~BranchNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */