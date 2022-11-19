#ifndef BRANCH_PATH_H
#define BRANCH_PATH_H

class BranchPath {
public:
	std::vector<AbstractScope*> scopes;

	std::vector<bool> need_process;

	std::vector<std::vector<SmallNetwork*>> inner_input_networks;
	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<double> scope_average_mod;
	std::vector<double> scope_scale_mod;

	std::vector<bool> is_branch;
	std::vector<Branch*> branches;

	std::vector<SubFoldNetwork*> score_networks;

	std::vector<int> compress_sizes;
	std::vector<bool> active_compress;
	std::vector<SubFoldNetwork*> compress_networks;

	int end_input_size;
	FoldNetwork* end_input_network;

	int explore_index_inclusive;	// -1 if not on path
	int explore_type;
	int explore_end_non_inclusive;
	Fold* explore_fold;
};

#endif /* BRANCH_PATH_H */