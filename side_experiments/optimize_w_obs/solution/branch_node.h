#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

#include <fstream>
#include <vector>

class Network;
class Problem;
class ScopeHistory;
class Solution;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	/**
	 * - world model makes it easier to make future improvements(?)
	 *   - BranchNode won't be dependent on specific previous nodes/paths
	 */
	bool is_local;
	std::vector<std::pair<std::pair<std::vector<int>,std::vector<int>>,int>> inputs;
	Network* network;

	std::vector<std::vector<int>> input_scope_context_ids;
	std::vector<std::vector<int>> input_node_context_ids;
	/**
	 * - 1.0 if branch, -1.0 if original
	 */

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

	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void new_scope_activate(AbstractNode*& curr_node,
							Problem* problem,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							ScopeHistory* scope_history);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	void new_scope_capture_verify_activate(AbstractNode*& curr_node,
										   Problem* problem,
										   std::vector<ContextLayer>& context,
										   RunHelper& run_helper,
										   ScopeHistory* scope_history);
	void clear_verify();
	#endif /* MDEBUG */

	void clean_inputs(int scope_id,
					  int node_id);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory(BranchNode* node);
	BranchNodeHistory(BranchNodeHistory* original);
};

#endif /* BRANCH_NODE_H */