#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

const int INPUT_TYPE_STATE = 0;
const int INPUT_TYPE_CONSTANT = 1;

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;

	std::vector<int> starting_node_ids;

	std::vector<int> input_types;
	std::vector<int> input_inner_layers;
	std::vector<bool> input_inner_is_local;
	std::vector<int> input_inner_ids;
	std::vector<bool> input_outer_is_local;
	std::vector<int> input_outer_ids;
	std::vector<double> input_init_vals;
	/**
	 * - don't worry about reversing signs
	 *   - can only be an issue with perfect XORs
	 *     - otherwise, can align state polarity when constructing
	 *   - makes it difficult to squash sequences into new scopes
	 */

	/**
	 * - from input states
	 */
	std::vector<int> output_inner_ids;
	std::vector<bool> output_outer_is_local;
	std::vector<int> output_outer_ids;

	std::vector<bool> state_is_local;
	std::vector<int> state_ids;
	std::vector<int> state_obs_indexes;
	std::vector<State*> state_defs;
	std::vector<int> state_network_indexes;

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	std::vector<int> score_state_obs_indexes;
	std::vector<State*> score_state_defs;
	std::vector<int> score_state_network_indexes;

	std::vector<std::vector<int>> experiment_hook_score_state_scope_contexts;
	std::vector<std::vector<int>> experiment_hook_score_state_node_contexts;
	std::vector<int> experiment_hook_score_state_obs_indexes;
	std::vector<State*> experiment_hook_score_state_defs;
	std::vector<int> experiment_hook_score_state_network_indexes;

	std::vector<int> test_hook_scope_contexts;
	std::vector<int> test_hook_node_contexts;
	int test_hook_obs_index;
	int test_hook_index;

	int next_node_id;



};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	bool is_halfway;



	std::map<int, StateStatus> obs_snapshots;


};

#endif /* SCOPE_NODE_H */