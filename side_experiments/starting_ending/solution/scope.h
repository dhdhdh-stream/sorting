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
class ActionNode;
class PassThroughExperiment;
class State;

class ScopeHistory;
class Scope {
public:
	int id;

	int num_input_states;
	int num_local_states;

	int node_counter;
	std::map<int, AbstractNode*> nodes;

	int starting_node_id;
	AbstractNode* starting_node;

	std::vector<State*> temp_states;
	std::vector<std::vector<ActionNode*>> temp_state_nodes;
	std::vector<std::vector<std::vector<int>>> temp_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> temp_state_node_contexts;
	std::vector<std::vector<int>> temp_state_obs_indexes;
	std::vector<int> temp_state_new_local_indexes;

	Scope();
	~Scope();

	void activate(Problem& problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void random_activate(std::vector<Scope*>& scope_context,
						 std::vector<AbstractNode*>& node_context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 std::vector<AbstractNode*>& possible_nodes,
						 std::vector<std::vector<Scope*>>& possible_scope_contexts,
						 std::vector<std::vector<AbstractNode*>>& possible_node_contexts);

	void view_activate(Problem& problem,
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

	AbstractExperiment* inner_experiment;

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