/**
 * - if original, 1.0; if branch, -1.0
 */

#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<int> branch_scope_context;
	std::vector<int> branch_node_context;
	/**
	 * - last layer of context doesn't matter
	 *   - scope id will always match, and node id meaningless
	 */
	bool branch_is_pass_through;

	double original_score_mod;
	double branch_score_mod;

	std::vector<bool> decision_state_is_local;
	std::vector<int> decision_state_indexes;
	std::vector<double> decision_original_weights;
	std::vector<double> decision_branch_weights;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	BranchNode();
	~BranchNode();

	void activate(bool& is_branch,
				  std::vector<ContextLayer>& context);

	void random_activate(bool& is_branch,
						 std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 int& num_nodes,
						 std::vector<AbstractNodeHistory*>& node_histories);

	void view_activate(bool& is_branch,
					   std::vector<ContextLayer>& context);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */