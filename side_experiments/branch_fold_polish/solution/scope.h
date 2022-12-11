// TODO: don't deep copy, but only deep copy score networks? add an extra layer of abstraction?

#ifndef SCOPE_H
#define SCOPE_H

const int STEP_TYPE_STEP = 0;
const int STEP_TYPE_BRANCH = 1;
const int STEP_TYPE_FOLD = 2;

class Scope {
public:
	int num_inputs;
	int num_outputs;	// cannot be 0 aside from starting/outer scope, which won't be included in dictionary

	std::vector<bool> is_inner_scope;
	std::vector<Scope*> scopes;
	std::vector<int> obs_sizes;	// TODO: exchange with raw actions later

	std::vector<std::vector<FoldNetwork*>> inner_input_networks;
	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<double> scope_scale_mod;
	// mods to bootstrap flat, but score networks after flat to adjust end

	std::vector<int> step_types;
	std::vector<Branch*> branches;
	// don't need branch mods after flat as all scores will be updated
	std::vector<Fold*> folds;

	std::vector<FoldNetwork*> score_networks;
	// outer solution will hold on to final outer score_network (and no compress needed)

	std::vector<double> average_misguesses;	// track also after branches
	std::vector<double> average_inner_scope_impacts;
	std::vector<double> average_local_impacts;	// if scope end, will be 0.0, so don't explore
	std::vector<double> average_inner_branch_impacts;
	// 50% of the time, explore inner -- otherwise, explore local
	// TODO: compare against parent impact to determine explore weight
	// TODO: explore when predicted_score drops and hit explore weight

	std::vector<bool> active_compress;
	std::vector<int> compress_new_sizes;	// may be expansion instead of compression because of folds
	std::vector<FoldNetwork*> compress_networks;
	std::vector<int> compress_original_sizes;	// for backprop

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
	std::vector<FoldHistory*> fold_histories;
	std::vector<FoldNetworkHistory*> score_network_histories;
	// TODO: in FoldNetworkHistory, don't save output
	std::vector<double> score_updates;
	std::vector<FoldNetworkHistory*> compress_network_histories;

	FoldHistory* explore_fold_history;
};

#endif /* SCOPE_H */