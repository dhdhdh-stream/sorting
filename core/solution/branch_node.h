#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

class RetrainBranchExperiment;

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

	double decision_standard_deviation;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	RetrainBranchExperiment* experiment;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	std::vector<std::vector<double>> verify_factors;
	#endif /* MDEBUG */

	BranchNode();
	~BranchNode();

	void activate(bool& is_branch,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void random_activate(bool& is_branch,
						 std::vector<Scope*>& scope_context,
						 std::vector<AbstractNode*>& node_context,
						 std::vector<AbstractNode*>& possible_nodes,
						 std::vector<std::vector<Scope*>>& possible_scope_contexts,
						 std::vector<std::vector<AbstractNode*>>& possible_node_contexts);
	void random_exit_activate(bool& is_branch,
							  std::vector<int>& scope_context,
							  std::vector<int>& node_context,
							  int curr_depth,
							  std::vector<std::pair<int,AbstractNode*>>& possible_exits);

	void view_activate(bool& is_branch,
					   std::vector<ContextLayer>& context);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(Problem* problem,
						 bool& is_branch,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	#endif /* MDEBUG */

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void success_reset();
	void fail_reset();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	BranchNodeHistory(BranchNode* node);
	BranchNodeHistory(BranchNodeHistory* original);
};

#endif /* BRANCH_NODE_H */