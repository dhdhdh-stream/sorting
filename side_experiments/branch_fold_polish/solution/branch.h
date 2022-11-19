#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	std::vector<SubFoldNetwork*> score_networks;
	std::vector<BranchPath*> branches;

	// no start_mods, just rely on score_networks instead

	std::vector<int> compress_sizes;
	std::vector<bool> active_compress;
	std::vector<Network*> compress_networks;

	std::vector<double> end_average_mods;
	std::vector<double> end_scale_mods;

	// used for backprop
	// TODO: create a backprop_history abstraction for this
	double best_index;
};

#endif /* BRANCH_H */