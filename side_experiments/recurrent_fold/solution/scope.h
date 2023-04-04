#ifndef SCOPE_H
#define SCOPE_H

#include <vector>

#include "abstract_node.h"
#include "fold.h"
#include "loop_fold.h"
#include "state_network.h"

class FoldHistory;
class LoopFoldHistory;
class ScopeHistory;
class Scope {
public:
	int id;

	int num_local_states;
	int num_input_states;	// also becomes output

	bool is_loop;
	std::vector<StateNetwork*> starting_state_networks;	// first local states, as num_local_states may later be incremented
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

	Scope(int num_local_states,
		  int num_input_states,
		  bool is_loop,
		  std::vector<StateNetwork*> starting_state_networks,
		  StateNetwork* continue_score_network,
		  StateNetwork* continue_misguess_network,
		  StateNetwork* halt_score_network,
		  StateNetwork* halt_misguess_network,
		  std::vector<AbstractNode*> nodes);
	Scope(std::ifstream& input_file);
	~Scope();

	void activate(std::vector<double>& input_vals,
				  std::vector<std::vector<double>>& flat_vals,
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
	void backprop(std::vector<double>& input_errors,
				  double target_val,
				  double final_misguess,
				  double final_sum_impact,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	bool handle_node_activate_helper(int iter_index,
									 int& curr_node_id,
									 FoldHistory*& curr_fold_history,
									 std::vector<double>& local_state_vals,
									 std::vector<double>& input_vals,
									 std::vector<std::vector<double>>& flat_vals,
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
									 std::vector<double>& local_state_errors,
									 std::vector<double>& input_errors,
									 double target_val,
									 double final_misguess,
									 double final_sum_impact,
									 double& predicted_score,
									 double& scale_factor,
									 RunHelper& run_helper,
									 ScopeHistory* history);
	void backprop_explore_fold_helper(std::vector<double>& local_state_errors,
									  std::vector<double>& input_errors,
									  double target_val,
									  double final_misguess,
									  double& predicted_score,
									  double& scale_factor,
									  RunHelper& run_helper,
									  ScopeHistory* history);

	void new_outer_to_local(int new_outer_size);
	void new_outer_to_input(int new_outer_size);

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

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */