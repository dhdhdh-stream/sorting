// Note: BranchPath fields are similar to Scope, but methods are different

#ifndef BRANCH_PATH_H
#define BRANCH_PATH_H

#include <fstream>
#include <vector>

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

	std::vector<double> average_scores;
	std::vector<double> average_misguesses;
	std::vector<double> average_inner_scope_impacts;
	std::vector<double> average_local_impacts;
	std::vector<double> average_inner_branch_impacts;

	std::vector<bool> active_compress;
	std::vector<int> compress_new_sizes;
	std::vector<FoldNetwork*> compress_networks;
	std::vector<int> compress_original_sizes;

	bool full_last;

	std::vector<int> starting_state_sizes;

	int explore_type;
	int explore_index_inclusive;
	int explore_end_non_inclusive;
	Fold* explore_fold;
	int explore_count;	// TODO: reset if too high

	BranchPath(int num_inputs,
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
			   std::vector<double> average_scores,
			   std::vector<double> average_misguesses,
			   std::vector<double> average_inner_scope_impacts,
			   std::vector<double> average_local_impacts,
			   std::vector<double> average_inner_branch_impacts,
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
								  double& scale_factor,
								  BranchPathHistory* history);
	void explore_off_path_backprop(std::vector<double>& local_s_input_errors,
								   std::vector<double>& local_state_errors,
								   double& predicted_score,
								   double target_val,
								   double& scale_factor,
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
						 double& next_predicted_score,
						 double target_val,
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
	void update_increment(BranchPathHistory* history);

	void explore_replace();
	void explore_branch();
	void resolve_fold(int a_index);

	void save(std::ofstream& output_file);
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

	int explore_type;
	int explore_index_inclusive;
	int explore_end_non_inclusive;
	int sequence_length;
	std::vector<bool> is_existing;
	std::vector<Scope*> existing_actions;
	std::vector<Action> actions;

	BranchPathHistory(BranchPath* branch_path);
	~BranchPathHistory();
};

#endif /* BRANCH_PATH_H */