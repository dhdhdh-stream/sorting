#ifndef BRANCH_H
#define BRANCH_H

#include <fstream>
#include <vector>

#include "branch_path.h"
#include "fold.h"
#include "fold_network.h"
#include "problem.h"
#include "run_status.h"

class BranchPath;
class Branch {
public:
	int id;

	int num_inputs;
	int num_outputs;
	int outer_s_input_size;

	FoldNetwork* branch_score_network;
	bool passed_branch_score;	// if branch_score_network taken by outer branch

	std::vector<FoldNetwork*> score_networks;	// don't update predicted_score until on branch_path
	std::vector<bool> is_branch;
	std::vector<BranchPath*> branches;
	std::vector<Fold*> folds;

	int explore_ref_count;

	Branch(int num_inputs,
		   int num_outputs,
		   int outer_s_input_size,
		   FoldNetwork* branch_score_network,
		   std::vector<FoldNetwork*> score_networks,
		   std::vector<bool> is_branch,
		   std::vector<BranchPath*> branches,
		   std::vector<Fold*> folds);
	Branch(std::ifstream& input_file);
	~Branch();

	void explore_on_path_activate_score(std::vector<double>& local_s_input_vals,
										std::vector<double>& local_state_vals,
										double& scale_factor,
										RunStatus& run_status,
										BranchHistory* history);
	void explore_off_path_activate_score(std::vector<double>& local_s_input_vals,
										 std::vector<double>& local_state_vals,
										 double& scale_factor,
										 RunStatus& run_status,
										 BranchHistory* history);
	void explore_on_path_activate(Problem& problem,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  RunStatus& run_status,
								  BranchHistory* history);
	void explore_off_path_activate(Problem& problem,
								   std::vector<double>& local_s_input_vals,
								   std::vector<double>& local_state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   RunStatus& run_status,
								   BranchHistory* history);
	void explore_on_path_backprop(std::vector<double>& local_s_input_errors,
								  std::vector<double>& local_state_errors,
								  double& predicted_score,
								  double target_val,
								  double final_misguess,
								  double& scale_factor,
								  double& scale_factor_error,
								  BranchHistory* history);
	void explore_off_path_backprop(std::vector<double>& local_s_input_errors,
								   std::vector<double>& local_state_errors,
								   double& predicted_score,
								   double target_val,
								   double& scale_factor,
								   double& scale_factor_error,
								   BranchHistory* history);
	void existing_flat_activate(Problem& problem,
								std::vector<double>& local_s_input_vals,
								std::vector<double>& local_state_vals,
								double& predicted_score,
								double& scale_factor,
								RunStatus& run_status,
								BranchHistory* history);
	void existing_flat_backprop(std::vector<double>& local_s_input_errors,
								std::vector<double>& local_state_errors,
								double& predicted_score,
								double predicted_score_error,
								double& scale_factor,
								double& scale_factor_error,
								BranchHistory* history);
	void update_activate(Problem& problem,
						 std::vector<double>& local_s_input_vals,
						 std::vector<double>& local_state_vals,
						 double& predicted_score,
						 double& scale_factor,
						 RunStatus& run_status,
						 BranchHistory* history);
	void update_backprop(double& predicted_score,
						 double target_val,
						 double final_misguess,
						 double& scale_factor,
						 double& scale_factor_error,
						 BranchHistory* history);
	void existing_update_activate(Problem& problem,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  RunStatus& run_status,
								  BranchHistory* history);
	void existing_update_backprop(double& predicted_score,
								  double predicted_score_error,
								  double& scale_factor,
								  double& scale_factor_error,
								  BranchHistory* history);

	void explore_set(BranchHistory* history);
	void explore_clear(BranchHistory* history);
	void update_increment(BranchHistory* history,
						  std::vector<Fold*>& folds_to_delete);

	void resolve_fold(int b_index,
					  std::vector<Fold*>& folds_to_delete);

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file,
						  int curr_scope_id);
};

class BranchPathHistory;
class BranchHistory {
public:
	Branch* branch;

	double best_score;	// scaled
	int best_index;

	FoldNetworkHistory* branch_score_network_history;
	double branch_score_update;

	FoldNetworkHistory* score_network_history;
	BranchPathHistory* branch_path_history;
	FoldHistory* fold_history;

	BranchHistory(Branch* branch);
	~BranchHistory();
};

#endif /* BRANCH_H */