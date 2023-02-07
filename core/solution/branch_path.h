#ifndef BRANCH_PATH_H
#define BRANCH_PATH_H

#include <fstream>
#include <vector>

#include "action.h"
#include "branch.h"
#include "fold.h"
#include "fold_network.h"
#include "network.h"
#include "problem.h"
#include "run_status.h"
#include "scope.h"

class Branch;
class BranchPathHistory;
class Fold;
class Scope;
class BranchPath {
public:
	int id;

	int num_inputs;
	int num_outputs;
	int outer_s_input_size;

	int sequence_length;
	// don't special case starting step in fields
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

	std::vector<double> average_inner_scope_impacts;
	std::vector<double> average_local_impacts;
	std::vector<double> average_inner_branch_impacts;

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	std::vector<bool> active_compress;
	std::vector<int> compress_new_sizes;
	std::vector<FoldNetwork*> compress_networks;
	std::vector<int> compress_original_sizes;

	bool full_last;

	std::vector<int> starting_state_sizes;

	int explore_type;
	bool explore_is_try;
	int explore_index_inclusive;
	int explore_end_non_inclusive;

	int explore_sequence_length;
	std::vector<int> explore_new_sequence_types;
	std::vector<int> explore_existing_scope_ids;
	std::vector<int> explore_existing_action_ids;
	std::vector<Action> explore_new_actions;

	Fold* explore_fold;
	int explore_count;	// TODO: reset if too high

	BranchPath(int num_inputs,
			   int num_outputs,
			   int outer_s_input_size,
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
	BranchPath(std::ifstream& input_file);
	~BranchPath();

	void explore_on_path_activate(Problem& problem,
								  double starting_score,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  RunStatus& run_status,
								  BranchPathHistory* history);
	void explore_off_path_activate(Problem& problem,
								   double starting_score,
								   std::vector<double>& local_s_input_vals,
								   std::vector<double>& local_state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   RunStatus& run_status,
								   BranchPathHistory* history);
	void explore_on_path_backprop(std::vector<double>& local_s_input_errors,
								  std::vector<double>& local_state_errors,
								  double& predicted_score,
								  double target_val,
								  double final_misguess,
								  double& scale_factor,
								  double& scale_factor_error,
								  BranchPathHistory* history);
	void explore_off_path_backprop(std::vector<double>& local_s_input_errors,
								   std::vector<double>& local_state_errors,
								   double& predicted_score,
								   double target_val,
								   double& scale_factor,
								   double& scale_factor_error,
								   BranchPathHistory* history);
	void existing_flat_activate(Problem& problem,
								double starting_score,
								std::vector<double>& local_s_input_vals,
								std::vector<double>& local_state_vals,
								double& predicted_score,
								double& scale_factor,
								RunStatus& run_status,
								BranchPathHistory* history);
	void existing_flat_backprop(std::vector<double>& local_s_input_errors,
								std::vector<double>& local_state_errors,
								double& predicted_score,
								double predicted_score_error,
								double& scale_factor,
								double& scale_factor_error,
								BranchPathHistory* history);
	void update_activate(Problem& problem,
						 double starting_score,
						 std::vector<double>& local_s_input_vals,
						 std::vector<double>& local_state_vals,
						 double& predicted_score,
						 double& scale_factor,
						 RunStatus& run_status,
						 BranchPathHistory* history);
	void update_backprop(double& predicted_score,
						 double target_val,
						 double final_misguess,
						 double& scale_factor,
						 double& scale_factor_error,
						 BranchPathHistory* history);
	void existing_update_activate(Problem& problem,
								  double starting_score,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  RunStatus& run_status,
								  BranchPathHistory* history);
	void existing_update_backprop(double& predicted_score,
								  double predicted_score_error,
								  double& scale_factor,
								  double& scale_factor_error,
								  BranchPathHistory* history);

	void explore_set(BranchPathHistory* history);
	void explore_clear(BranchPathHistory* history);
	void update_increment(BranchPathHistory* history,
						  std::vector<Fold*>& folds_to_delete);

	void explore_replace();
	void explore_branch();
	void resolve_fold(int a_index,
					  std::vector<Fold*>& folds_to_delete);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file,
						  int curr_scope_id);
};

class BranchHistory;
class FoldHistory;
class ScopeHistory;
class BranchPathHistory {
public:
	BranchPath* branch_path;

	std::vector<std::vector<FoldNetworkHistory*>> inner_input_network_histories;
	std::vector<ScopeHistory*> scope_histories;
	std::vector<BranchHistory*> branch_histories;
	std::vector<FoldHistory*> fold_histories;
	std::vector<FoldNetworkHistory*> score_network_histories;
	std::vector<double> score_updates;
	std::vector<FoldNetworkHistory*> compress_network_histories;
	
	FoldHistory* explore_fold_history;

	int exit_index;
	int exit_location;

	BranchPathHistory(BranchPath* branch_path);
	~BranchPathHistory();
};

#endif /* BRANCH_PATH_H */