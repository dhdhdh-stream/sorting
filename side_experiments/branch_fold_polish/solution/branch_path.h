// Note: BranchPath fields are identical to Scope, but methods are different

#ifndef BRANCH_PATH_H
#define BRANCH_PATH_H

class BranchPath {
public:
	// don't special case starting step in fields
	std::vector<bool> is_inner_scope;
	std::vector<Scope*> scopes;
	std::vector<int> obs_sizes;

	std::vector<std::vector<FoldNetwork*>> inner_input_networks;
	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<double> scope_scale_mod;

	std::vector<bool> step_types;
	std::vector<Branch*> branches;
	std::vector<Fold*> folds;

	std::vector<FoldNetwork*> score_networks;
	// use NULL score_network to signify was scope end and no further processing needed

	std::vector<double> average_misguesses;
	std::vector<double> average_inner_scope_impacts;
	std::vector<double> average_local_impacts;
	std::vector<double> average_inner_branch_impacts;

	std::vector<bool> active_compress;
	std::vector<int> compress_new_sizes;
	std::vector<FoldNetwork*> compress_networks;
	std::vector<int> compress_original_sizes;

	int explore_index_inclusive;
	int explore_type;
	int explore_end_non_inclusive;
	Fold* explore_fold;

	BranchPath(std::vector<bool> is_inner_scope,
			   std::vector<Scope*> scopes,
			   std::vector<int> obs_sizes,
			   std::vector<std::vector<FoldNetwork*>> inner_input_networks,
			   std::vector<std::vector<int>> inner_input_sizes,
			   std::vector<double> scope_scale_mod,
			   std::vector<bool> step_types,
			   std::vector<Branch*> branches,
			   std::vector<Fold*> folds,
			   std::vector<FoldNetwork*> score_networks,
			   std::vector<double> average_misguesses,
			   std::vector<double> average_inner_scope_impacts,
			   std::vector<double> average_local_impacts,
			   std::vector<double> average_inner_branch_impacts,
			   std::vector<bool> active_compress,
			   std::vector<int> compress_new_sizes,
			   std::vector<FoldNetwork*> compress_networks,
			   std::vector<int> compress_original_sizes);
	~BranchPath();


};

class BranchPathHistory {
public:
	BranchPath* branch_path;

	std::vector<std::vector<FoldNetworkHistory*>> inner_input_network_histories;
	std::vector<ScopeHistory*> scope_histories;
	std::vector<BranchHistory*> branch_histories;
	std::vector<FoldHistory*> fold_histories;
	std::vector<FoldNetworkHistory*> score_network_histories;
	std::vector<double> score_updates;
	std::vector<FoldNetworkHistory*> compress_network_histories;
	
	FoldNetworkHistory* end_input_network_history;

	FoldHistory* explore_fold_history;
};

#endif /* BRANCH_PATH_H */