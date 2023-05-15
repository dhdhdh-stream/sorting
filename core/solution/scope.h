#ifndef SCOPE_H
#define SCOPE_H

#include <vector>

#include "abstract_node.h"
#include "fold.h"
#include "loop_fold.h"
#include "problem.h"
#include "run_helper.h"
#include "state_network.h"

class FoldHistory;
class LoopFoldHistory;
class RunHelper;
class ScopeHistory;
class Scope {
public:
	int id;

	// TODO: name states (i.e., objects), so that on reuse in exploration, can immediately make intelligent choices
	//   - will need to track states globally
	//   - but try other states too
	//     - if successful, link state to each other
	//       - so build state graph as well
	//       - link states by attributes, and for actions, build what attributes are desired for what inputs
	//   - probably not needed for sorting, but needed for bigger problems

	// - can't really slot in other states though because not correctly scaled?
	//   - have to use the same state, but track globally?
	//   - can use other state if already related
	//   - so explore decision is just whether to reuse existing global state, or blank (potentially introduce new related state)

	// - also track state that appear together
	//   - states that are used in the same network
	//   - they will be related in a different way
	//     - when one of them is used, others should be used too

	// - keeping track of initialization can be good because:
	//   - if initialized, add copy of object
	//   - otherwise, keep modifying object initialized from outside

	// - add L1/L2, and also check the update size of networks, and use to quick filter
	//   - use L1 norm here for feature removal

	// - use just in time for calculating state?
	//   - likely saves a lot of effort as network gets big?
	//   - main question is on experiment, what state to include
	//     - should ideally be everything initially
	//       - so iterate through and fire every network?
	//     - what version of state to use?
	//       - either last or near to last?
	//       - or line up with iterations
	//         - but humans don't remember lists

	// - perhaps actions aren't just actual actions, but include looking through memory
	//   - solves lots of problems:
	//     - just in time
	//     - which version of state to use
	//     - what state to include for experiment, score network, etc.
	//       - shouldn't need to worry about outer state anyways since what enabled high impact should already almost be good enough
	// - if state needed for action, just run the fetch beforehand

	// - don't freeze state anymore?
	//   - modifying existing state for new purposes

	// - how to split folds?
	//   - the thing is, when an outer state is created, and it could be extended into inner, won't be sure if outer and inner can be combined, or should be separate
	//     - only way to know for sure is if scope is reused, and realize outer state needs to be extended for reuse
	//       - so maybe don't even have inner state?

	// - maybe don't try to do everything at once
	//   - focus on a single score, a single branch each time

	// - no, scopes are still good
	//   - a segment where a certain set of data is useful
	//     - maybe when folding, split/clear first, then remove after
	//   - so don't distinguish between outer and inner state?

	// - maybe priority is to use the minimum amount of state possible
	//   - for state, don't have earlier depending on later, but make it hierarchical/sequential
	//     - regularization, and split into scopes

	// - don't split score and sequence

	// - don't split between replace and branch, but just include misguess networks?
	//   - actually no, more efficient to just replace

	// - hierarchy/layering for rewind is just having dependencies
	//   - so states can also have dependencies that can change
	//     - again, units for the dependencies have to be the same

	// - if there is a recursive dependency, then inner depends on outer
	//   - else if there is only dependency in one direction, then inner can simply be independent

	int num_states;
	std::vector<bool> is_initialized_locally;	// for folds, try even if initialized locally -- will instead initialize outside in fold

	// Note: don't backprop halt/continue networks after train, simply hope that backproping score helps
	//   - because no easy way to determine continue target
	bool is_loop;
	std::vector<StateNetwork*> starting_state_networks;	// first states
	StateNetwork* continue_score_network;
	StateNetwork* continue_misguess_network;
	StateNetwork* halt_score_network;
	StateNetwork* halt_misguess_network;
	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;
	int furthest_successful_halt;

	std::vector<AbstractNode*> nodes;

	Scope(int num_states,
		  std::vector<bool> is_initialized_locally,
		  bool is_loop,
		  std::vector<StateNetwork*> starting_state_networks,
		  StateNetwork* continue_score_network,
		  StateNetwork* continue_misguess_network,
		  StateNetwork* halt_score_network,
		  StateNetwork* halt_misguess_network,
		  double average_score,
		  double score_variance,
		  double average_misguess,
		  double misguess_variance,
		  std::vector<AbstractNode*> nodes);
	Scope(std::ifstream& input_file);
	~Scope();

	void activate(Problem& problem,
				  std::vector<double>& state_vals,
				  std::vector<bool>& inputs_initialized,
				  double& predicted_score,
				  double& scale_factor,
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  std::vector<ScopeHistory*>& context_histories,
				  int& early_exit_depth,
				  int& early_exit_node_id,
				  FoldHistory*& early_exit_fold_history,
				  int& explore_exit_depth,
				  int& explore_exit_node_id,
				  FoldHistory*& explore_exit_fold_history,
				  RunHelper& run_helper,
				  ScopeHistory* history);
	void backprop(std::vector<double>& state_errors,
				  std::vector<bool>& inputs_initialized,
				  double target_val,
				  double final_diff,
				  double final_misguess,
				  double& predicted_score,
				  double& scale_factor,
				  double& scale_factor_error,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	bool handle_node_activate_helper(int iter_index,
									 int& curr_node_id,
									 FoldHistory*& curr_fold_history,
									 Problem& problem,
									 std::vector<double>& state_vals,
									 std::vector<bool>& states_initialized,
									 double& predicted_score,
									 double& scale_factor,
									 std::vector<int>& scope_context,
									 std::vector<int>& node_context,
									 std::vector<ScopeHistory*>& context_histories,
									 int& early_exit_depth,
									 int& early_exit_node_id,
									 FoldHistory*& early_exit_fold_history,
									 int& explore_exit_depth,
									 int& explore_exit_node_id,
									 FoldHistory*& explore_exit_fold_history,
									 RunHelper& run_helper,
									 ScopeHistory* history);
	void handle_node_backprop_helper(int iter_index,
									 int h_index,
									 std::vector<double>& state_errors,
									 std::vector<bool>& states_initialized,
									 double target_val,
									 double final_diff,
									 double final_misguess,
									 double& predicted_score,
									 double& scale_factor,
									 double& scale_factor_error,
									 RunHelper& run_helper,
									 ScopeHistory* history);

	void explore_new_loop(int curr_node_id,
						  Problem& problem,
						  std::vector<double>& state_vals,
						  std::vector<bool>& states_initialized,
						  double& predicted_score,
						  std::vector<int>& scope_context,
						  std::vector<int>& node_context,
						  std::vector<ScopeHistory*>& context_histories,
						  RunHelper& run_helper);
	void explore_new_path(int curr_node_id,
						  int next_node_id,
						  Problem& problem,
						  std::vector<double>& state_vals,
						  std::vector<bool>& states_initialized,
						  double& predicted_score,
						  double& scale_factor,
						  std::vector<int>& scope_context,
						  std::vector<int>& node_context,
						  std::vector<ScopeHistory*>& context_histories,
						  int& new_explore_exit_depth,
						  int& new_explore_exit_node_id,
						  RunHelper& run_helper);
	void backprop_explore_fold_helper(std::vector<double>& state_errors,
									  std::vector<bool>& states_initialized,
									  double target_val,
									  double final_diff,
									  double final_misguess,
									  double& predicted_score,
									  double& scale_factor,
									  double& scale_factor_error,
									  RunHelper& run_helper,
									  ScopeHistory* history);

	void add_new_state(int new_state_size,
					   bool initialized_locally);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<StateNetworkHistory*> starting_state_network_histories;

	int num_loop_iters;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	int explore_iter_index;
	int explore_node_index;
	FoldHistory* explore_fold_history;
	LoopFoldHistory* explore_loop_fold_history;

	bool exceeded_depth;

	ScopeHistory(Scope* scope);
	ScopeHistory(ScopeHistory* original);	// deep copy for seed
	~ScopeHistory();
};

#endif /* SCOPE_H */