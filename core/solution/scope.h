/**
 * TODO: to handle loops, average values for flat into index network
 * - for flat even if only one iter is relevant, and that iter changes, will still be correlation
 * - for index network:
 *   - keep track of 2 states, a val state, and an index state
 *     - val state directly tied to ending score as usual
 *     - index state tied to time
 *       - minus by 1.0 each iteration so roughly acts as a counter until state update is needed
 *       - measure activation*index to normalize
 *   - have 4 sub-networks:
 *     - a val state update network as usual
 *       - can depend on self
 *     - an index state update network
 *       - depends on obs, but not on self
 *     - a forget network to erase index state
 *       - implemented as summing negative of index state times sigmoid
 *         - so error signals still pass through
 *       - initialize output constant to -10.0 so never forgets initially
 *     - an update size network to control val state update size
 *       - sigmoid times state update
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <list>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"
#include "state_status.h"

class AbstractNode;
class AbstractNodeHistory;
class BranchExperimentHistory;
class ObsExperiment;
class Sequence;
class State;

class ScopeHistory;
class Scope {
public:
	int id;

	int num_input_states;
	int num_local_states;

	std::map<int, AbstractNode*> nodes;

	std::list<ScopeHistory*> scope_histories;
	std::list<std::map<int, StateStatus>> input_state_vals_histories;
	std::list<std::map<int, StateStatus>> local_state_vals_histories;
	std::list<double> target_val_histories;

	std::vector<double> input_state_weights;
	std::vector<double> local_state_weights;

	/**
	 * - clear after every update
	 */
	std::vector<State*> score_states;
	std::vector<std::vector<AbstractNode*>> score_state_nodes;
	std::vector<std::vector<std::vector<int>>> score_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> score_state_node_contexts;
	std::vector<std::vector<int>> score_state_obs_indexes;
	std::vector<double> score_state_weights;

	double average_score;
	double score_variance;
	/**
	 * - measure using sqr over abs
	 *   - even though sqr may not measure true score improvement, it measures information improvement
	 *     - which ultimately leads to better branching
	 */
	double average_misguess;
	double misguess_variance;

	std::vector<Scope*> child_scopes;
	/**
	 * - don't remove even if can no longer reach
	 *   - might have been a mistaken pass_through anyways
	 */

	Scope();
	~Scope();

	void activate(std::vector<int>& starting_node_ids,
				  std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
				  std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
				  Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  int& exit_node_id,
				  RunHelper& run_helper,
				  ScopeHistory* history);
	void node_activate_helper(int iter_index,
							  int& curr_node_id,
							  Problem& problem,
							  std::vector<ContextLayer>& context,
							  int& exit_depth,
							  int& exit_node_id,
							  RunHelper& run_helper,
							  ScopeHistory* history);

	void random_activate(std::vector<int>& starting_node_ids,
						 std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 int& exit_depth,
						 int& exit_node_id,
						 int& num_nodes,
						 ScopeHistory* history);
	void node_random_activate_helper(int& curr_node_id,
									 std::vector<int>& scope_context,
									 std::vector<int>& node_context,
									 int& exit_depth,
									 int& exit_node_id,
									 int& num_nodes,
									 ScopeHistory* history);
	void random_exit_activate(std::vector<int>& starting_node_ids,
							  std::vector<int>& scope_context,
							  std::vector<int>& node_context,
							  int& exit_depth,
							  int& exit_node_id,
							  int& num_nodes,
							  ScopeHistory* history);
	void node_random_exit_activate_helper(int& curr_node_id,
										  std::vector<int>& scope_context,
										  std::vector<int>& node_context,
										  int& exit_depth,
										  int& exit_node_id,
										  int& num_nodes,
										  ScopeHistory* history);

	void create_sequence_activate(std::vector<int>& starting_node_ids,
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
	void node_create_sequence_activate_helper(int& curr_node_id,
											  Problem& problem,
											  std::vector<ContextLayer>& context,
											  int target_num_nodes,
											  int& curr_num_nodes,
											  Sequence* new_sequence,
											  std::vector<std::map<std::pair<bool,int>, int>>& state_mappings,
											  int& new_num_input_states,
											  std::vector<AbstractNode*>& new_nodes,
											  RunHelper& run_helper);

	void view_activate(std::vector<int>& starting_node_ids,
					   std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
					   std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
					   Problem& problem,
					   std::vector<ContextLayer>& context,
					   int& exit_depth,
					   int& exit_node_id,
					   RunHelper& run_helper);
	void node_view_activate_helper(int iter_index,
								   int& curr_node_id,
								   Problem& problem,
								   std::vector<ContextLayer>& context,
								   int& exit_depth,
								   int& exit_node_id,
								   RunHelper& run_helper);

	void update_histories(double target_val,
						  ScopeHistory* history);
	void update();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  int id);

	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	bool exceeded_depth;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */