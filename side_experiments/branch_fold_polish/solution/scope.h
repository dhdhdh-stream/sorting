#ifndef SCOPE_H
#define SCOPE_H

const int SCOPE_TYPE_BASE = 0;
const int SCOPE_TYPE_SCOPE = 1;

class AbstractScope {
public:
	int type;

	virtual ~AbstractScope() {};
};

class BaseScope : public AbstractScope {
public:
	int num_outputs;

	BaseScope(int num_outputs);
	~BaseScope();
};

class Scope : public AbstractScope {
public:
	std::string id;

	int num_inputs;
	int num_outputs;

	std::vector<int> start_scope_input_input_indexes;

	// start can't be inherited_score
	std::vector<AbstractScope*> scopes;

	std::vector<std::vector<FoldNetwork*>> inner_input_networks;
	std::vector<std::vector<int>> inner_input_sizes;
	// TODO: initialize mods to 0.0 and 1.0
	std::vector<Network*> scope_average_mod;
	std::vector<Network*> scope_scale_mod;
	// scope end average adjusted by score network

	// always need_process, as observations might become relevant later
	// std::vector<bool> need_process;

	std::vector<bool> is_branch;
	std::vector<Branch*> branches;
	// for when retrain front (i.e., all averages will need to be modified down as overall score increases)
	std::vector<Network*> branch_average_mod;
	std::vector<Network*> branch_scale_mod;

	std::vector<FoldNetwork*> score_networks;

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
	std::vector<double> score_updates;
	std::vector<FoldNetworkHistory*> compress_network_histories;

	FoldHistory* fold_history;
};

#endif /* SCOPE_H */