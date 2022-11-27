#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	FoldNetwork* branch_score_network;
	bool passed_branch_score;	// if branch_score_network taken by outer branch

	bool does_inherit;
	std::vector<FoldNetwork*> score_networks;	// don't update predicted_score until on branch_path
	std::vector<bool> is_branch;
	std::vector<BranchPath*> branches;
	std::vector<Fold*> folds;
	// no differing end_indexes, but instead, multiple score_networks per branch

	// TODO: when exploring, branches will not find flats at once, but whenever one does,
	// go upwards, and freeze
	// reference counts flats downwards, and if all fail, move on
	// if success, can keep reference counts and keep going? (because of folds, might be more likely to go to 0) (can also just reset after a certain number of successes)

	// maybe if notice observations growing in weight, good time to redo? but XORs can't be captured, so maybe just redo every so often anyways

	// for folding a branch with the front, for the other branches' input, add output networks (like the opposite of the input networks currently)
};

class BranchHistory {
public:
	Branch* branch;

	double best_score;
	int best_index;

	FoldNetworkHistory* branch_score_network_history;
	double branch_score;

	FoldNetworkHistory* score_network_history;
	BranchPathHistory* branch_path_history;
	FoldHistory* fold_history;
};

#endif /* BRANCH_H */