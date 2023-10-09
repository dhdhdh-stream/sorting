/**
 * - if original, 1.0; if branch, -1.0
 * 
 * - only have score networks, no misguess networks
 *   - difficult to maintain over branching anyways
 */

#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"
#include "state_status.h"

class Sequence;
class State;
class BranchExperiment;
class BranchExperimentHistory;

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

	std::vector<bool> shared_state_is_local;
	std::vector<int> shared_state_indexes;
	std::vector<double> branch_weights;
	std::vector<State*> original_state_defs;

	std::vector<bool> branch_state_is_local;
	std::vector<int> branch_state_indexes;
	std::vector<State*> branch_state_defs;

	int branch_next_node_id;
	int original_next_node_id;

	std::vector<bool> state_is_local;
	std::vector<int> state_indexes;
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

	BranchNode();
	BranchNode(std::ifstream& input_file,
			   int id);
	~BranchNode();

	void activate(int& curr_node_id,
				  std::vector<double>& flat_vals,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  int& exit_node_id,
				  RunHelper& run_helper,
				  std::vector<AbstractNodeHistory*>& node_histories);

	void create_sequence_activate(bool& is_branch,
								  std::vector<ContextLayer>& context,
								  int target_num_nodes,
								  int& curr_num_nodes,
								  Sequence* new_sequence,
								  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
								  int& new_num_input_states,
								  std::vector<AbstractNode*>& new_nodes);

	void random_activate(bool& is_branch,
						 std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 int& num_nodes,
						 std::vector<AbstractNodeHistory*>& node_histories);
	void random_exit_activate(bool& is_branch,
							  std::vector<int>& scope_context,
							  std::vector<int>& node_context,
							  int& num_nodes,
							  std::vector<AbstractNodeHistory*>& node_histories);

	void experiment_back_activate(std::vector<int>& scope_context,
								  std::vector<int>& node_context,
								  std::map<State*, StateStatus>& experiment_score_state_vals,
								  std::vector<int>& test_obs_indexes,
								  std::vector<double>& test_obs_vals,
								  BranchNodeHistory* history);

	void save(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	std::vector<int> score_state_indexes;
	std::vector<StateStatus> score_state_impacts;

	BranchExperimentHistory* branch_experiment_history;

	BranchNodeHistory(BranchNode* node);
	~BranchNodeHistory();
};

#endif /* BRANCH_NODE_H */