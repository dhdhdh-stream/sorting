/**
 * - if original, 1.0; if branch, -1.0
 * 
 * - only have score networks, no misguess networks
 *   - difficult to maintain over branching anyways
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

	std::vector<int> score_ids;

	std::vector<double> branch_score_weights;
	int branch_next_node_id;

	std::vector<double> original_score_weights;
	int original_next_node_id;

	std::vector<int> local_state_ids;
	std::vector<int> obs_ids;
	std::vector<State*> states;
	std::vector<int> network_indexes;

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	/**
	 * - for top scope context
	 */
	std::vector<State*> score_states;
	std::vector<int> score_obs_ids;
	std::vector<int> score_network_indexes;

	std::vector<std::vector<int>> experiment_hook_score_state_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_score_state_node_contexts;
	std::vector<State*> experiment_hook_score_states;
	std::vector<int> experiment_hook_score_obs_ids;
	std::vector<int> experiment_hook_score_network_indexes;

	std::vector<int> test_hook_scope_contexts;
	std::vector<int> test_hook_node_contexts;
	int test_hook_obs_id;
	int test_hook_index;

	bool experiment_is_branch;
	AbstractExperiment* experiment;



};

#endif /* BRANCH_NODE_H */