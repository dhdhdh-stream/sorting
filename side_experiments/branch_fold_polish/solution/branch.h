#ifndef BRANCH_H
#define BRANCH_H

#include <fstream>
#include <vector>

#include "branch_path.h"
#include "fold.h"
#include "fold_network.h"

class BranchPath;
class Branch {
public:
	int id;

	FoldNetwork* branch_score_network;
	bool passed_branch_score;	// if branch_score_network taken by outer branch

	std::vector<FoldNetwork*> score_networks;	// don't update predicted_score until on branch_path
	std::vector<bool> is_branch;
	std::vector<BranchPath*> branches;
	std::vector<Fold*> folds;

	// TODO: when exploring, branches will not find flats at once, but whenever one does,
	// go upwards, and freeze
	// reference counts flats downwards, and if all fail, move on
	// if success, can keep reference counts and keep going? (because of folds, might be more likely to go to 0) (can also just reset after a certain number of successes)

	// TODO: for folding a branch with the front, for the other branches' input, add output networks (like the opposite of the input networks currently)
	// set one full path as primary, and fold completely?
	// - there will be branches coming off, but can try to match existing
	//   - during flat (and later during fold), if not taking branch, add state to mark
	// - if branch travelled, set unhit to blank
	// so can still be made a simple flat into a simple fold?

	Branch(FoldNetwork* branch_score_network,
		   std::vector<FoldNetwork*> score_networks,
		   std::vector<bool> is_branch,
		   std::vector<BranchPath*> branches,
		   std::vector<Fold*> folds);
	Branch(Branch* original);
	Branch(std::ifstream& input_file);
	~Branch();

	void explore_on_path_activate_score(std::vector<double>& local_s_input_vals,
										std::vector<double>& local_state_vals,
										double& scale_factor,
										int& explore_phase,
										BranchHistory* history);
	void explore_off_path_activate_score(std::vector<double>& local_s_input_vals,
										 std::vector<double>& local_state_vals,
										 double& scale_factor,
										 int& explore_phase,
										 BranchHistory* history);
	void explore_on_path_activate(std::vector<std::vector<double>>& flat_vals,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  int& explore_phase,
								  BranchHistory* history);
	void explore_off_path_activate(std::vector<std::vector<double>>& flat_vals,
								   std::vector<double>& local_s_input_vals,
								   std::vector<double>& local_state_vals,
								   double& predicted_score,
								   double& scale_factor,
								   int& explore_phase,
								   BranchHistory* history);
	void explore_on_path_backprop(std::vector<double>& local_s_input_errors,
								  std::vector<double>& local_state_errors,
								  double& predicted_score,
								  double target_val,
								  double& scale_factor,
								  BranchHistory* history);
	void explore_off_path_backprop(std::vector<double>& local_s_input_errors,
								   std::vector<double>& local_state_errors,
								   double& predicted_score,
								   double target_val,
								   double& scale_factor,
								   BranchHistory* history);
	void existing_flat_activate(std::vector<std::vector<double>>& flat_vals,
								std::vector<double>& local_s_input_vals,
								std::vector<double>& local_state_vals,
								double& predicted_score,
								double& scale_factor,
								BranchHistory* history);
	void existing_flat_backprop(std::vector<double>& local_s_input_errors,
								std::vector<double>& local_state_errors,
								double& predicted_score,
								double predicted_score_error,
								double& scale_factor,
								double& scale_factor_error,
								BranchHistory* history);
	void update_activate(std::vector<std::vector<double>>& flat_vals,
						 std::vector<double>& local_s_input_vals,
						 std::vector<double>& local_state_vals,
						 double& predicted_score,
						 double& scale_factor,
						 BranchHistory* history);
	void update_backprop(double& predicted_score,
						 double& next_predicted_score,
						 double target_val,
						 double& scale_factor,
						 BranchHistory* history);
	void existing_update_activate(std::vector<std::vector<double>>& flat_vals,
								  std::vector<double>& local_s_input_vals,
								  std::vector<double>& local_state_vals,
								  double& predicted_score,
								  double& scale_factor,
								  BranchHistory* history);
	void existing_update_backprop(double& predicted_score,
								  double predicted_score_error,
								  double& scale_factor,
								  double& scale_factor_error,
								  BranchHistory* history);

	void resolve_fold(int b_index);

	void save(std::ofstream& output_file);
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