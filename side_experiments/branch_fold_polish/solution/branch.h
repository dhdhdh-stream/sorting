#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	FoldNetwork* branch_score_network;
	bool passed_branch_score;	// if branch_score_network taken by outer branch

	std::vector<FoldNetwork*> score_networks;	// don't update predicted_score until on branch_path
	std::vector<bool> is_branch;
	std::vector<BranchPath*> branches;
	std::vector<Fold*> folds;
	std::vector<double> end_scale_mods;

	bool is_scope_end;

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
		   std::vector<Fold*> folds,
		   std::vector<double> end_scale_mods,
		   bool is_scope_end);
	~Branch();


};

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
};

#endif /* BRANCH_H */