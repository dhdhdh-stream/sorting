#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"
#include "state_status.h"

class Scope;
class ScopeHistory;
class Sequence;
class State;
class BranchExperiment;
class BranchExperimentHistory;

const int INPUT_TYPE_STATE = 0;
const int INPUT_TYPE_CONSTANT = 1;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	Scope* inner_scope;
	/**
	 * - use id when saving/loading, but have direct reference for running
	 */

	std::vector<int> starting_node_ids;

	std::vector<int> input_types;
	std::vector<int> input_inner_layers;
	std::vector<bool> input_inner_is_local;
	std::vector<int> input_inner_indexes;
	std::vector<bool> input_outer_is_local;
	std::vector<int> input_outer_indexes;
	std::vector<double> input_init_vals;
	/**
	 * - don't worry about reversing signs
	 *   - can only be an issue with perfect XORs
	 *     - otherwise, can align state polarity when constructing
	 *   - makes it difficult to squash sequences into new scopes
	 */

	/**
	 * - from input states
	 *   - inner local states impact outside through obs/state networks
	 */
	std::vector<int> output_inner_indexes;
	std::vector<bool> output_outer_is_local;
	std::vector<int> output_outer_indexes;

	std::vector<bool> state_is_local;
	std::vector<int> state_indexes;
	std::vector<int> state_obs_indexes;
	std::vector<State*> state_defs;
	std::vector<int> state_network_indexes;

	std::vector<std::vector<int>> temp_state_scope_contexts;
	std::vector<std::vector<int>> temp_state_node_contexts;
	std::vector<int> temp_state_obs_indexes;
	std::vector<State*> temp_state_defs;
	std::vector<int> temp_state_network_indexes;

	std::vector<std::vector<int>> experiment_state_scope_contexts;
	std::vector<std::vector<int>> experiment_state_node_contexts;
	std::vector<int> experiment_state_obs_indexes;
	std::vector<State*> experiment_state_defs;
	std::vector<int> experiment_state_network_indexes;

	int next_node_id;

	AbstractExperiment* experiment;

	std::vector<std::vector<int>> obs_experiment_scope_contexts;
	std::vector<std::vector<int>> obs_experiment_node_contexts;
	std::vector<int> obs_experiment_obs_indexes;
	std::vector<int> obs_experiment_indexes;

	ScopeNode();
	ScopeNode(std::ifstream& input_file,
			  int id);
	~ScopeNode();

	void activate(int& curr_node_id,
				  Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  int& exit_node_id,
				  RunHelper& run_helper,
				  std::vector<AbstractNodeHistory*>& node_histories);
	void halfway_activate(std::vector<int>& starting_node_ids,
						  std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
						  std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
						  int& curr_node_id,
						  Problem& problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  int& exit_node_id,
						  RunHelper& run_helper,
						  std::vector<AbstractNodeHistory*>& node_histories);

	void random_activate(std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 int& inner_exit_depth,
						 int& inner_exit_node_id,
						 int& num_nodes,
						 std::vector<AbstractNodeHistory*>& node_histories);
	void halfway_random_activate(std::vector<int>& starting_node_ids,
								 std::vector<int>& scope_context,
								 std::vector<int>& node_context,
								 int& inner_exit_depth,
								 int& inner_exit_node_id,
								 int& num_nodes,
								 std::vector<AbstractNodeHistory*>& node_histories);

	void create_sequence_activate(Problem& problem,
								  std::vector<ContextLayer>& context,
								  int target_num_nodes,
								  int& curr_num_nodes,
								  Sequence* new_sequence,
								  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
								  int& new_num_input_states,
								  std::vector<AbstractNode*>& new_nodes,
								  RunHelper& run_helper);
	void halfway_create_sequence_activate(std::vector<int>& starting_node_ids,
										  std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
										  std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
										  std::vector<std::map<std::pair<bool,int>, int>>& starting_state_mappings,
										  Problem& problem,
										  std::vector<ContextLayer>& context,
										  int target_num_nodes,
										  int& curr_num_nodes,
										  Sequence* new_sequence,
										  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
										  int& new_num_input_states,
										  std::vector<AbstractNode*>& new_nodes,
										  RunHelper& run_helper);

	void flat_vals_back_activate(std::vector<int>& scope_context,
								 std::vector<int>& node_context,
								 int d_index,
								 int stride_size,
								 std::vector<double>& flat_vals,
								 ScopeNodeHistory* history);
	void rnn_vals_back_activate(std::vector<int>& scope_context,
								std::vector<int>& node_context,
								std::vector<int>& obs_indexes,
								std::vector<double>& obs_vals,
								ScopeNodeHistory* history);
	void experiment_back_activate(std::vector<int>& scope_context,
								  std::vector<int>& node_context,
								  std::map<int, StateStatus>& experiment_score_state_vals,
								  ScopeNodeHistory* history);

	void view_activate(int& curr_node_id,
					   Problem& problem,
					   std::vector<ContextLayer>& context,
					   int& exit_depth,
					   int& exit_node_id,
					   RunHelper& run_helper);
	void halfway_view_activate(std::vector<int>& starting_node_ids,
							   std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
							   std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
							   int& curr_node_id,
							   Problem& problem,
							   std::vector<ContextLayer>& context,
							   int& exit_depth,
							   int& exit_node_id,
							   RunHelper& run_helper);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	bool is_halfway;
	bool is_early_exit;

	ScopeHistory* inner_scope_history;

	std::map<int, StateStatus> obs_snapshots;

	AbstractExperimentHistory* experiment_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */