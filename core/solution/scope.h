/**
 * TODO: to handle loops, average values, and possibly select one iter/context to focus on
 * - even if only one iter is relevant, and that iter changes, will still be correlation
 * 
 * - perhaps average for flat, but then train as usual for RNN
 *   - TODO: try depth 100 RNN where only 1 iter matters?
 *     - though will probably end up effectively averaging just like flat
 *       - maybe this is where LSTM/focus matters?
 *         - or no, would require extra state?
 *           - with extra state, LSTM could work
 *             - use 1 extra state as goal is just indexing
 * 
 * - have 1 network blindly generate potential updates based only on obs and val state
 * - the LSTM part doesn't see/use val at all
 * - have another network blindly update index state using only obs and index state
 * - have a separate predicted score that is updated
 * - have 3rd network based on index state determine how much previous val state matters to 1st network
 * - have 4th network based on index state determine how much impact update has
 * 
 * - will need some modification as LSTM is meant to output values every timestep
 */

#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <map>
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

	/**
	 * - no need to explicitly track score states here
	 */

	std::vector<AbstractNode*> nodes;

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
	 * - for constructing new sequences
	 */

	ObsExperiment* obs_experiment;

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

	void update_backprop(double target_val,
						 ScopeHistory* history);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  int id);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	std::map<State*, StateStatus> score_state_snapshots;

	// for ObsExperiment
	std::vector<int> test_obs_indexes;
	std::vector<double> test_obs_vals;

	BranchExperimentHistory* inner_branch_experiment_history;

	bool exceeded_depth;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */