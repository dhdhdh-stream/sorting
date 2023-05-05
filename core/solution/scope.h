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

	// TODO: actually, name states (i.e., objects), so that on reuse in exploration, can immediately make intelligent choices
	int num_states;
	std::vector<bool> is_initialized_locally;	// for folds, try even if initialized locally -- will instead initialize outside in fold

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
				  double& sum_impact,
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
				  double final_misguess,
				  double final_sum_impact,
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
									 double& sum_impact,
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
									 double final_misguess,
									 double final_sum_impact,
									 double& predicted_score,
									 double& scale_factor,
									 double& scale_factor_error,
									 RunHelper& run_helper,
									 ScopeHistory* history);

	void explore_new_loop(int curr_node_id,
						  Problem& problem,
						  double& predicted_score,
						  std::vector<int>& scope_context,
						  std::vector<int>& node_context,
						  std::vector<ScopeHistory*>& context_histories,
						  RunHelper& run_helper);
	void explore_new_path(int curr_node_id,
						  int next_node_id,
						  Problem& problem,
						  std::vector<double>& state_vals,
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
									  double final_misguess,
									  double final_sum_impact,
									  double& predicted_score,
									  double& scale_factor,
									  double& scale_factor_error,
									  RunHelper& run_helper,
									  ScopeHistory* history);

	void add_new_state(int new_state_size,
					   bool initialized_locally);

	void save(std::ofstream& output_file);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<StateNetworkHistory*> starting_state_network_histories;

	int num_loop_iters;

	std::vector<double> continue_score_network_updates;
	std::vector<StateNetworkHistory*> continue_score_network_histories;
	std::vector<double> continue_misguess_vals;
	std::vector<StateNetworkHistory*> continue_misguess_network_histories;
	double halt_score_network_update;
	StateNetworkHistory* halt_score_network_history;
	double halt_misguess_val;
	StateNetworkHistory* halt_misguess_network_history;

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