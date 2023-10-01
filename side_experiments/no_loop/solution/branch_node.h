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

	std::vector<bool> shared_state_is_local;
	std::vector<int> shared_state_ids;
	std::vector<double> branch_weights;
	std::vector<State*> original_state_defs;

	std::vector<bool> branch_state_is_local;
	std::vector<int> branch_state_ids;
	std::vector<State*> branch_state_defs;

	int branch_next_node_id;
	int original_next_node_id;

	std::vector<bool> state_is_local;
	std::vector<int> state_ids;
	std::vector<State*> state_defs;
	std::vector<int> state_network_indexes;

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	std::vector<State*> score_state_defs;
	std::vector<int> score_state_network_indexes;

	std::vector<std::vector<int>> experiment_hook_score_state_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_score_state_node_contexts;
	std::vector<State*> experiment_hook_score_state_defs;
	std::vector<int> experiment_hook_score_state_network_indexes;

	std::vector<std::vector<int>> test_hook_scope_contexts;
	std::vector<std::vector<int>> test_hook_node_contexts;
	std::vector<int> test_hook_indexes;
	std::vector<void*> test_hook_keys;

	BranchExperiment* experiment;
	bool experiment_is_branch;
	/**
	 * - only trigger if on right branch
	 */



};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	std::vector<int> score_state_indexes;
	std::vector<StateStatus> score_state_impacts;

	BranchExperimentHistory* branch_experiment_history;

};

#endif /* BRANCH_NODE_H */