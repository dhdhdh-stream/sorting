#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

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

	AbstractExperiment* experiment;
	bool experiment_is_branch;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_original_scores;
	std::vector<double> verify_branch_scores;
	#endif /* MDEBUG */


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