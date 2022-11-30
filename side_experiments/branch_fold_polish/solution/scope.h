#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	std::string id;

	int num_inputs;
	int num_outputs;

	std::vector<bool> is_inner_scope;
	std::vector<Scope*> scopes;
	std::vector<int> obs_sizes;	// TODO: exchange with raw actions later

	std::vector<std::vector<FoldNetwork*>> inner_input_networks;
	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<double> scope_scale_mod;
	// mods to bootstrap flat, but score networks after flat to adjust end

	std::vector<bool> is_branch;
	std::vector<Branch*> branches;
	// don't need branch mods after flat as all scores will be updated

	std::vector<FoldNetwork*> score_networks;
	// outer solution will hold on to final outer score_network (and no compress needed)

	std::vector<double> average_misguesses;	// track also after branches

	std::vector<bool> active_compress;
	std::vector<int> compress_sizes;
	std::vector<FoldNetwork*> compress_networks;

	int explore_index_inclusive;
	int explore_type;
	int explore_end_non_inclusive;
	Fold* explore_fold;

};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<std::vector<FoldNetworkHistory*>> inner_input_network_histories;
	std::vector<ScopeHistory*> scope_histories;
	std::vector<BranchHistory*> branch_histories;
	std::vector<FoldNetworkHistory*> score_network_histories;
	// TODO: in FoldNetworkHistory, don't save output
	std::vector<double> score_updates;
	std::vector<FoldNetworkHistory*> compress_network_histories;

	FoldHistory* fold_history;
};

#endif /* SCOPE_H */