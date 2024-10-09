#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

#include <fstream>
#include <vector>

class Network;
class Problem;
class Solution;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<std::pair<std::pair<std::vector<int>,std::vector<int>>,int>> inputs;
	Network* network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	BranchNode();
	BranchNode(BranchNode* original);
	~BranchNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void result_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void new_action_capture_verify_activate(AbstractNode*& curr_node,
											Problem* problem,
											std::vector<ContextLayer>& context,
											RunHelper& run_helper);
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;
};

#endif /* BRANCH_NODE_H */