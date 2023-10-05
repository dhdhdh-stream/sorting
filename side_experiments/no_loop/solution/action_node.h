#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"
#include "state_status.h"

class BranchExperiment;
class BranchExperimentHistory;
class Sequence;
class State;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<bool> state_is_local;
	std::vector<int> state_indexes;
	std::vector<State*> state_defs;
	/**
	 * - use id when saving/loading, but have direct reference for running
	 */
	std::vector<int> state_network_indexes;

	std::vector<std::vector<int>> score_state_scope_contexts;
	std::vector<std::vector<int>> score_state_node_contexts;
	/**
	 * - for top scope context
	 */
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

	int next_node_id;

	BranchExperiment* experiment;

	ActionNode();
	ActionNode(std::ifstream& input_file,
			   int id);
	~ActionNode();

	void activate(int& curr_node_id,
				  std::vector<double>& flat_vals,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  int& exit_node_id,
				  RunHelper& run_helper,
				  std::vector<AbstractNodeHistory*>& node_histories);

	void create_sequence_activate(std::vector<double>& flat_vals,
								  std::vector<ContextLayer>& context,
								  int target_num_nodes,
								  int& curr_num_nodes,
								  Sequence* new_sequence,
								  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
								  int& new_num_input_states,
								  std::vector<AbstractNode*>& new_nodes);

	void branch_experiment_train_activate(
		std::vector<double>& flat_vals,
		std::vector<ContextLayer>& context);
	void branch_experiment_simple_activate(
		std::vector<double>& flat_vals);

	void experiment_back_activate(std::vector<int>& scope_context,
								  std::vector<int>& node_context,
								  std::map<State*, StateStatus>& experiment_score_state_vals,
								  std::vector<int>& test_obs_indexes,
								  std::vector<double>& test_obs_vals,
								  ActionNodeHistory* history);

	void save(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	std::vector<int> score_state_indexes;
	std::vector<StateStatus> score_state_impacts;

	BranchExperimentHistory* branch_experiment_history;

	ActionNodeHistory(ActionNode* node);
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */