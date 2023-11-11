#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"
#include "state_status.h"

class AbstractExperiment;
class AbstractExperimentHistory;
class Sequence;
class State;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	Action action;

	std::vector<bool> state_is_local;
	std::vector<int> state_indexes;
	std::vector<State*> state_defs;
	/**
	 * - use id when saving/loading, but have direct reference for running
	 */
	std::vector<int> state_network_indexes;

	std::vector<std::vector<int>> temp_state_scope_contexts;
	std::vector<std::vector<int>> temp_state_node_contexts;
	std::vector<State*> temp_state_defs;
	std::vector<int> temp_state_network_indexes;

	/**
	 * - is also temp_state but keep separate for easier hook/clear
	 */
	std::vector<std::vector<int>> experiment_state_scope_contexts;
	std::vector<std::vector<int>> experiment_state_node_contexts;
	std::vector<State*> experiment_state_defs;
	std::vector<int> experiment_state_network_indexes;

	int next_node_id;
	AbstractNode* next_node;

	AbstractExperiment* experiment;
	/**
	 * - don't worry about possibility of having both PassThroughExperiment and BranchExperiment
	 */

	/**
	 * - hook
	 */
	std::vector<int> obs_experiment_scope_contexts;
	std::vector<int> obs_experiment_node_contexts;
	int obs_experiment_index;

	ActionNode();
	~ActionNode();

	void activate(AbstractNode*& curr_node,
				  Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);

	void create_sequence_activate(Problem& problem,
								  std::vector<ContextLayer>& context,
								  int target_num_nodes,
								  int& curr_num_nodes,
								  Sequence* new_sequence,
								  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
								  int& new_num_input_states,
								  std::vector<AbstractNode*>& new_nodes);

	void flat_vals_back_activate(std::vector<int>& scope_context,
								 std::vector<int>& node_context,
								 int d_index,
								 int stride_size,
								 std::vector<double>& flat_vals,
								 ActionNodeHistory* history);
	void rnn_vals_back_activate(std::vector<int>& scope_context,
								std::vector<int>& node_context,
								std::vector<int>& obs_indexes,
								std::vector<double>& obs_vals,
								ActionNodeHistory* history);
	void experiment_back_activate(std::vector<int>& scope_context,
								  std::vector<int>& node_context,
								  std::map<State*, StateStatus>& temp_state_vals,
								  ActionNodeHistory* history);

	void view_activate(AbstractNode*& curr_node,
					   Problem& problem,
					   std::vector<ContextLayer>& context,
					   int& exit_depth,
					   AbstractNode*& exit_node,
					   RunHelper& run_helper);

	void success_reset();
	void fail_reset();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;

	AbstractExperimentHistory* experiment_history;

	ActionNodeHistory(ActionNode* node);
	ActionNodeHistory(ActionNodeHistory* original);
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */