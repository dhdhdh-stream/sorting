#ifndef SCOPE_H
#define SCOPE_H

#include <fstream>
#include <vector>

#include "action.h"
#include "branch.h"
#include "fold.h"
#include "fold_network.h"
#include "network.h"
#include "problem.h"
#include "run_status.h"

class Branch;
class Fold;
class ScopeHistory;
class Scope {
public:
	int id;

	int num_inputs;
	int num_outputs;

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<Scope*> scopes;
	std::vector<Action> actions;

	std::vector<std::vector<FoldNetwork*>> inner_input_networks;
	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<Network*> scope_scale_mod;

	std::vector<int> step_types;
	std::vector<Branch*> branches;
	std::vector<Fold*> folds;

	std::vector<FoldNetwork*> score_networks;
	// Note: don't use soft targets even with early exit, as hard targets needed to determine impact

	std::vector<double> average_inner_scope_impacts;
	std::vector<double> average_local_impacts;	// if scope end, will be 0.0, so don't explore
	std::vector<double> average_inner_branch_impacts;

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	std::vector<bool> active_compress;
	std::vector<int> compress_new_sizes;	// may be expansion instead of compression because of folds
	std::vector<FoldNetwork*> compress_networks;
	std::vector<int> compress_original_sizes;	// WARN: may not actually match original size due to fold to scope transformation -- use with compress_new_sizes

	bool full_last;

	// to help construct folds
	std::vector<int> starting_state_sizes;

	int explore_type;
	int explore_index_inclusive;

	int explore_curr_try;
	int explore_target_tries;
	double best_explore_surprise;
	int best_explore_end_non_inclusive;
	int best_explore_sequence_length;
	std::vector<int> best_explore_new_sequence_types;
	std::vector<int> best_explore_existing_scope_ids;
	std::vector<int> best_explore_existing_action_ids;
	std::vector<Action> best_explore_new_actions;
	std::vector<double> best_seed_local_s_input_vals;
	std::vector<double> best_seed_local_state_vals;
	double best_seed_start_score;
	double best_seed_target_val;

	int curr_explore_end_non_inclusive;
	int curr_explore_sequence_length;
	std::vector<int> curr_explore_new_sequence_types;
	std::vector<int> curr_explore_existing_scope_ids;
	std::vector<int> curr_explore_existing_action_ids;
	std::vector<Action> curr_explore_new_actions;
	std::vector<double> curr_seed_local_s_input_vals;
	std::vector<double> curr_seed_local_state_vals;
	double curr_seed_start_score;

	Fold* explore_fold;
	int explore_count;	// TODO: reset if too high

	Scope();
	~Scope();

	void initialize(int num_inputs,
					int num_outputs,
					int sequence_length,
					std::vector<bool> is_inner_scope,
					std::vector<Scope*> scopes,
					std::vector<Action> actions,
					std::vector<std::vector<FoldNetwork*>> inner_input_networks,
					std::vector<std::vector<int>> inner_input_sizes,
					std::vector<Network*> scope_scale_mod,
					std::vector<int> step_types,
					std::vector<Branch*> branches,
					std::vector<Fold*> folds,
					std::vector<FoldNetwork*> score_networks,
					std::vector<double> average_inner_scope_impacts,
					std::vector<double> average_local_impacts,
					std::vector<double> average_inner_branch_impacts,
					double average_score,
					double score_variance,
					double average_misguess,
					double misguess_variance,
					std::vector<bool> active_compress,
					std::vector<int> compress_new_sizes,
					std::vector<FoldNetwork*> compress_networks,
					std::vector<int> compress_original_sizes,
					bool full_last);
	void load(std::ifstream& input_file);

	void explore_on_path_activate(Problem& problem,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  RunStatus& run_status,
								  ScopeHistory* history);
	void explore_off_path_activate(Problem& problem,
								   std::vector<double>& local_s_input_vals,
								   std::vector<double>& local_state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   RunStatus& run_status,
								   ScopeHistory* history);
	void explore_on_path_backprop(std::vector<double>& local_state_errors,
								  double& predicted_score,
								  double target_val,
								  double final_misguess,
								  double& scale_factor,
								  double& scale_factor_error,
								  ScopeHistory* history);
	void explore_off_path_backprop(std::vector<double>& local_state_errors,
								   std::vector<double>& local_s_input_errors,
								   double& predicted_score,
								   double target_val,
								   double& scale_factor,
								   double& scale_factor_error,
								   ScopeHistory* history);
	void existing_flat_activate(Problem& problem,
								std::vector<double>& local_s_input_vals,
								std::vector<double>& local_state_vals,
								double& predicted_score,
								double& scale_factor,
								RunStatus& run_status,
								ScopeHistory* history);
	void existing_flat_backprop(std::vector<double>& local_state_errors,
								std::vector<double>& local_s_input_errors,
								double& predicted_score,
								double predicted_score_error,
								double& scale_factor,
								double& scale_factor_error,
								ScopeHistory* history);
	void update_activate(Problem& problem,
						 std::vector<double>& local_s_input_vals,
						 std::vector<double>& local_state_vals,
						 double& predicted_score,
						 double& scale_factor,
						 RunStatus& run_status,
						 ScopeHistory* history);
	void update_backprop(double& predicted_score,
						 double target_val,
						 double final_misguess,
						 double& scale_factor,
						 double& scale_factor_error,
						 ScopeHistory* history);
	void existing_update_activate(Problem& problem,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  RunStatus& run_status,
								  ScopeHistory* history);
	void existing_update_backprop(double& predicted_score,
								  double predicted_score_error,
								  double& scale_factor,
								  double& scale_factor_error,
								  ScopeHistory* history);

	void explore_set(double target_val,
					 double existing_score,
					 ScopeHistory* history);
	// TODO: clean up update_increment logic
	void update_increment(ScopeHistory* history,
						  std::vector<Fold*>& folds_to_delete);

	void explore_replace();
	void explore_branch();
	void resolve_fold(int a_index,
					  std::vector<Fold*>& folds_to_delete);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class BranchHistory;
class FoldHistory;
class ScopeHistory {
public:
	Scope* scope;

	std::vector<std::vector<FoldNetworkHistory*>> inner_input_network_histories;
	std::vector<ScopeHistory*> scope_histories;
	std::vector<BranchHistory*> branch_histories;
	std::vector<FoldHistory*> fold_histories;
	std::vector<FoldNetworkHistory*> score_network_histories;
	std::vector<double> score_updates;
	std::vector<FoldNetworkHistory*> compress_network_histories;

	FoldHistory* explore_fold_history;

	// in case of early exit due to scope depth
	int exit_index;
	int exit_location;

	ScopeHistory(Scope* scope);
	~ScopeHistory();
};

#endif /* SCOPE_H */