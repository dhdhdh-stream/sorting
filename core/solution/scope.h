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

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	std::vector<Scope*> child_scopes;
	/**
	 * - don't remove even if can no longer reach
	 *   - might have been a mistaken change anyways
	 */

	std::vector<State*> temp_states;
	std::vector<std::vector<AbstractNode*>> temp_state_nodes;
	std::vector<std::vector<std::vector<int>>> temp_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> temp_state_node_contexts;
	std::vector<std::vector<int>> temp_state_obs_indexes;
	std::vector<int> temp_state_new_local_indexes;

	Scope();
	~Scope();

	void activate(std::vector<AbstractNode*>& starting_nodes,
				  std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
				  std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
				  Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void random_activate(std::vector<AbstractNode*>& starting_nodes,
						 std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 int& num_nodes,
						 ScopeHistory* history);

	void create_sequence_activate(std::vector<AbstractNode*>& starting_nodes,
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

	void view_activate(std::vector<AbstractNode*>& starting_nodes,
					   std::vector<std::map<int, StateStatus>>& starting_input_state_vals,
					   std::vector<std::map<int, StateStatus>>& starting_local_state_vals,
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

class ScopeHistory {
public:
	Scope* scope;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	PassThroughExperiment* inner_pass_through_experiment;

	int experiment_iter_index;
	int experiment_index;
	/**
	 * - for gathering possible_exits during measure_existing
	 */

	bool exceeded_depth;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */