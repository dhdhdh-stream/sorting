#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	std::vector<SubFoldNetwork*> score_networks;
	std::vector<BranchPath*> branches;

	std::vector<double> previous_average_mods;
	std::vector<double> previous_scale_mods;

	std::vector<int> compress_sizes;
	std::vector<bool> active_compress;
	std::vector<Network*> compress_networks;

	std::vector<double> ending_average_mods;
	std::vector<double> ending_scale_mods;
};

#endif /* BRANCH_H */