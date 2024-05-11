/**
 * - don't depend on history/traverse and don't be depended on by history/traverse
 *   - to make it cleaner to swap in and out
 */

#ifndef INFO_BRANCH_NODE_H
#define INFO_BRANCH_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class InfoScope;
class Problem;
class ScopeHistory;
class Solution;

class InfoBranchNodeHistory;
class InfoBranchNode : public AbstractNode {
public:
	InfoScope* scope;
	bool is_negate;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	InfoBranchNode();
	InfoBranchNode(InfoBranchNode* original,
				   Solution* parent_solution);
	~InfoBranchNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);
	#endif /* MDEBUG */

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class InfoBranchNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	bool is_branch;

	InfoBranchNodeHistory();
	InfoBranchNodeHistory(InfoBranchNodeHistory* original);
	~InfoBranchNodeHistory();
};

#endif /* INFO_BRANCH_NODE_H */