#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	std::vector<FoldNetwork*> score_networks;
	std::vector<BranchPath*> branches;

	// no start_mods, just rely on score_networks instead

	std::vector<int> compress_sizes;
	std::vector<bool> active_compress;
	std::vector<FoldNetwork*> compress_networks;

	std::vector<double> end_average_mods;
	std::vector<double> end_scale_mods;
};

class BranchHistory {
public:
	double best_index;

	FoldNetworkHistory* score_network_history;
	BranchPathHistory* branch_history;
	FoldNetworkHistory* compress_history;
};

#endif /* BRANCH_H */