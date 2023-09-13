#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

class BranchNode {
public:
	bool branch_is_pass_through

	ScoreNetwork* branch_score_network;
	ScoreNetwork* branch_misguess_network;
	int branch_next_node_id;

	ScoreNetwork* original_score_network;
	ScoreNetwork* original_misguess_network;
	int original_next_node_id;

	std::vector<int> state_indexes;
	std::vector<bool> state_is_branch;

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	/**
	 * - for top scope context
	 */
	std::vector<int> score_state_ids;
	std::vector<bool> score_state_is_branch;

	std::vector<int> experiment_hook_indexes;
	std::vector<std::vector<int>> experiment_hook_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_node_contexts;

	// TODO: besides on/off, still need normal score state networks

};

#endif /* BRANCH_NODE_H */