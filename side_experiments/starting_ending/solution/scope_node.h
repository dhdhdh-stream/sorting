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

const int INPUT_TYPE_STATE = 0;
const int INPUT_TYPE_CONSTANT = 1;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	Scope* inner_scope;

	std::vector<int> input_types;
	std::vector<int> input_inner_indexes;
	std::vector<bool> input_outer_is_local;
	std::vector<int> input_outer_indexes;
	std::vector<double> input_init_vals;
	/**
	 * - don't worry about reversing signs
	 *   - can only be an issue with perfect XORs
	 *     - otherwise, can align state polarity when constructing
	 */

	/**
	 * - from input states
	 */
	std::vector<int> output_inner_indexes;
	std::vector<bool> output_outer_is_local;
	std::vector<int> output_outer_indexes;

	int next_node_id;
	AbstractNode* next_node;

	AbstractExperiment* experiment;

	ScopeNode();
	~ScopeNode();

	void activate(AbstractNode*& curr_node,
				  Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ScopeNodeHistory* history);
	void halfway_activate(std::vector<AbstractNode*>& starting_nodes,
						  std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
						  std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
						  AbstractNode*& curr_node,
						  Problem& problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  ScopeNodeHistory* history);

	void random_activate(std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 int& inner_exit_depth,
						 AbstractNode*& inner_exit_node,
						 int& num_nodes,
						 std::vector<AbstractNodeHistory*>& node_histories);
	void halfway_random_activate(std::vector<AbstractNode*>& starting_nodes,
								 std::vector<int>& scope_context,
								 std::vector<int>& node_context,
								 int& inner_exit_depth,
								 AbstractNode*& inner_exit_node,
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
	void halfway_create_sequence_activate(std::vector<AbstractNode*>& starting_nodes,
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
								  std::map<State*, StateStatus>& experiment_score_state_vals,
								  ScopeNodeHistory* history);

	void view_activate(AbstractNode*& curr_node,
					   Problem& problem,
					   std::vector<ContextLayer>& context,
					   int& exit_depth,
					   AbstractNode*& exit_node,
					   RunHelper& run_helper);
	void halfway_view_activate(std::vector<AbstractNode*>& starting_nodes,
							   std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
							   std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
							   AbstractNode*& curr_node,
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

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* inner_scope_history;

	AbstractExperimentHistory* experiment_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */