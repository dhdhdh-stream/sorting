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
	std::vector<int> state_obs_indexes;
	std::vector<State*> state_defs;
	/**
	 * - use id when saving/loading, but have direct reference for running
	 */
	std::vector<int> state_network_indexes;

	std::vector<std::vector<int>> temp_state_scope_contexts;
	std::vector<std::vector<int>> temp_state_node_contexts;
	std::vector<int> temp_state_obs_indexes;
	std::vector<State*> temp_state_defs;
	std::vector<int> temp_state_network_indexes;

	/**
	 * - is also temp_state but keep separate for easier hook/clear
	 */
	std::vector<std::vector<int>> experiment_state_scope_contexts;
	std::vector<std::vector<int>> experiment_state_node_contexts;
	std::vector<int> experiment_state_obs_indexes;
	std::vector<State*> experiment_state_defs;
	std::vector<int> experiment_state_network_indexes;

	int next_node_id;
	AbstractNode* next_node;

	AbstractExperiment* experiment;

	/**
	 * - hook
	 */
	std::vector<std::vector<int>> obs_experiment_scope_contexts;
	std::vector<std::vector<int>> obs_experiment_node_contexts;
	std::vector<int> obs_experiment_obs_indexes;
	std::vector<int> obs_experiment_indexes;

	ActionNode();
	~ActionNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);

	void flat_vals_back_activate(std::vector<int>& scope_context,
								 std::vector<int>& node_context,
								 std::vector<double>& sum_vals,
								 std::vector<int>& counts,
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
					   Problem* problem,
					   std::vector<ContextLayer>& context,
					   int& exit_depth,
					   AbstractNode*& exit_node,
					   RunHelper& run_helper);

	void verify_activate(AbstractNode*& curr_node,
						 Problem* problem,
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

	std::vector<double> state_snapshots;

	AbstractExperimentHistory* experiment_history;

	ActionNodeHistory(ActionNode* node);
	ActionNodeHistory(ActionNodeHistory* original);
	~ActionNodeHistory();
};

#endif /* ACTION_NODE_H */