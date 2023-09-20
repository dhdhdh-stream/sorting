/**
 * - if original, 1.0; if branch, -1.0
 * 
 * - only have score networks, no misguess networks
 *   - would not maintain over branching anyways
 */

#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

class BranchNode : public AbstractNode {
public:
	std::vector<int> branch_scope_context;
	std::vector<int> branch_node_context;
	/**
	 * - last layer of context doesn't matter
	 *   - scope id will always match, and node id meaningless
	 */
	bool branch_is_pass_through

	std::vector<int> score_network_input_ids;

	ScoreNetwork* branch_score_network;
	int branch_next_node_id;

	ScoreNetwork* original_score_network;
	int original_next_node_id;

	std::vector<int> state_ids;
	std::vector<StateNetwork*> state_networks;

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	/**
	 * - for top scope context
	 */
	std::vector<int> score_state_ids;
	std::vector<StateNetwork*> score_state_networks;

	int experiment_hook_index;
	std::vector<int> experiment_hook_scope_contexts;
	std::vector<int> experiment_hook_node_contexts;



};

#endif /* BRANCH_NODE_H */