#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	bool does_inherit;
	std::vector<std::vector<FoldNetwork*>> score_networks;
	std::vector<bool> is_branch;
	std::vector<BranchPath*> branches;
	std::vector<Fold*> folds;
	// no differing end_indexes, but instead, multiple score_networks per branch

	// TODO: when exploring, branches will not find flats at once, but whenever one does,
	// go upwards, and freeze
	// reference counts flats downwards, and if all fail, move on
	// if success, can keep reference counts and keep going? (because of folds, might be more likely to go to 0) (can also just reset after a certain number of successes)
};

class BranchHistory {
public:
	double best_index;

	FoldNetworkHistory* score_network_history;
	BranchPathHistory* branch_history;
	FoldHistory* fold_history;
};

#endif /* BRANCH_H */