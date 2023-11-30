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

	bool is_loop;

	double continue_score_mod;
	double halt_score_mod;

	// loop_state_is_local = true
	std::vector<int> loop_state_indexes;
	std::vector<double> loop_continue_weights;
	std::vector<double> loop_halt_weights;

	int max_iters;

	std::vector<State*> temp_states;
	std::vector<std::vector<ActionNode*>> temp_state_nodes;
	std::vector<std::vector<std::vector<int>>> temp_state_scope_contexts;
	std::vector<std::vector<std::vector<int>>> temp_state_node_contexts;
	std::vector<std::vector<int>> temp_state_obs_indexes;
	std::vector<int> temp_state_new_local_indexes;

	void* verify_key;
	std::vector<double> verify_continue_scores;
	std::vector<double> verify_halt_scores;
	std::vector<std::vector<double>> verify_factors;
	std::vector<bool> verify_decision_is_halt;

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
	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<int>& scope_context,
							  std::vector<int>& node_context,
							  int& exit_depth,
							  AbstractNode*& exit_node,
							  int curr_depth,
							  std::vector<std::pair<int,AbstractNode*>>& possible_exits);

	void view_activate(Problem& problem,
					   std::vector<ContextLayer>& context,
					   int& exit_depth,
					   AbstractNode*& exit_node,
					   RunHelper& run_helper);

	void verify_activate(Problem& problem,
						 std::vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper);

	void success_reset();
	void fail_reset();

	void clear_verify();

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

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);
	~ScopeHistory();
};

#endif /* SCOPE_H */