#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

#include <fstream>
#include <vector>

class Network;
class PotentialCommit;
class Problem;
class ScopeHistory;
class Solution;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<std::pair<int,int>> factor_ids;
	std::vector<double> factor_weights;

	std::vector<std::vector<Scope*>> input_scope_contexts;
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
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 std::vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void new_scope_activate(AbstractNode*& curr_node,
							std::vector<ContextLayer>& context,
							RunHelper& run_helper,
							ScopeHistory* scope_history);

	void commit_gather_activate(AbstractNode*& curr_node,
								std::vector<ContextLayer>& context,
								RunHelper& run_helper,
								int& node_count,
								AbstractNode*& potential_node_context,
								bool& potential_is_branch);
	void commit_activate(AbstractNode*& curr_node,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 PotentialCommit* potential_commit);

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

	void clean_inputs(int node_id);
	void clean_inputs(Scope* scope);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory(BranchNode* node);
};

#endif /* BRANCH_NODE_H */