#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractExperiment;
class AbstractExperimentHistory;
class Network;
class Problem;
class Scope;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<int> scope_context_ids;
	std::vector<Scope*> scope_context;
	std::vector<int> node_context_ids;
	std::vector<AbstractNode*> node_context;

	bool is_pass_through;

	double original_average_score;
	double branch_average_score;

	/**
	 * - empty if input no longer accessible after create_scope()
	 *   - TODO: clean instead
	 */
	std::vector<std::vector<int>> input_scope_context_ids;
	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<int>> input_node_context_ids;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;

	std::vector<int> linear_original_input_indexes;
	std::vector<double> linear_original_weights;
	std::vector<int> linear_branch_input_indexes;
	std::vector<double> linear_branch_weights;

	std::vector<std::vector<int>> original_network_input_indexes;
	Network* original_network;
	std::vector<std::vector<int>> branch_network_input_indexes;
	Network* branch_network;

	/**
	 * - don't randomize decisions
	 *   - small variations in obs may lead to random-like behavior anyways
	 */

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	std::vector<int> hook_indexes;
	std::vector<std::vector<Scope*>> hook_scope_contexts;
	std::vector<std::vector<AbstractNode*>> hook_node_contexts;

	std::vector<AbstractExperiment*> experiments;
	std::vector<bool> experiment_is_branch;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */

	BranchNode();
	~BranchNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  std::vector<AbstractNodeHistory*>& node_histories);

	void back_activate(std::vector<Scope*>& scope_context,
					   std::vector<AbstractNode*>& node_context,
					   std::vector<double>& input_vals,
					   BranchNodeHistory* history);

	void random_activate(bool& is_branch,
						 std::vector<Scope*>& scope_context,
						 std::vector<AbstractNode*>& node_context,
						 std::vector<std::vector<Scope*>>& possible_scope_contexts,
						 std::vector<std::vector<AbstractNode*>>& possible_node_contexts);
	void random_exit_activate(bool& is_branch,
							  std::vector<Scope*>& scope_context,
							  std::vector<AbstractNode*>& node_context,
							  int curr_depth,
							  std::vector<std::pair<int,AbstractNode*>>& possible_exits);

	#if defined(MDEBUG) && MDEBUG
	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 std::vector<AbstractNodeHistory*>& node_histories);
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
	bool is_branch;

	AbstractExperimentHistory* experiment_history;

	BranchNodeHistory(BranchNode* node);
	BranchNodeHistory(BranchNodeHistory* original);
	~BranchNodeHistory();
};

#endif /* BRANCH_NODE_H */