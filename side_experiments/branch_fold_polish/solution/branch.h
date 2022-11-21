#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	// for when retrain front (i.e., average will need to be modified down as overall score increases)
	// TODO: when retrain front, only modify score networks
	double start_average_mod;
	double start_scale_mod;

	bool does_inherit;
	std::vector<std::vector<FoldNetwork*>> score_networks;
	std::vector<BranchPath*> branches;
	// no differing end_indexes, but instead, multiple score_networks per branch

	// no start_mods, just rely on score_networks instead

	std::vector<double> end_average_mods;
	std::vector<double> end_scale_mods;

	int explore_type;
	Fold* explore_fold;

};

class BranchHistory {
public:
	double best_index;

	FoldNetworkHistory* score_network_history;
	BranchPathHistory* branch_history;

	FoldHistory* fold_history;
};

#endif /* BRANCH_H */