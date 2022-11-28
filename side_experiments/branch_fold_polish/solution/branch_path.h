#ifndef BRANCH_PATH_H
#define BRANCH_PATH_H

class BranchPath {
public:
	// don't special case starting step in fields
	std::vector<AbstractScope*> scopes;
	std::vector<int> obs_sizes;

	std::vector<std::vector<FoldNetwork*>> inner_input_networks;
	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<double> scope_scale_mod;

	std::vector<bool> is_branch;
	std::vector<Branch*> branches;

	std::vector<FoldNetwork*> score_networks;

	std::vector<double> average_misguesses;

	std::vector<bool> active_compress;
	std::vector<int> compress_sizes;
	std::vector<FoldNetwork*> compress_networks;

	int end_input_size;
	FoldNetwork* end_input_network;

	double end_scale_mod;

	int explore_index_inclusive;
	int explore_type;
	int explore_end_non_inclusive;
	Fold* explore_fold;


};

class BranchPathHistory {
public:
	BranchPath* branch_path;

	std::vector<std::vector<FoldNetworkHistory*>> inner_input_network_histories;
	std::vector<ScopeHistory*> scope_histories;
	std::vector<BranchHistory*> branch_histories;
	std::vector<FoldNetworkHistory*> score_network_histories;
	std::vector<double> score_updates;
	std::vector<FoldNetworkHistory*> compress_network_histories;
	
	FoldNetworkHistory* end_input_network_history;
};

#endif /* BRANCH_PATH_H */