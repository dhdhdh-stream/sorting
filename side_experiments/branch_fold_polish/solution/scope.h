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

	std::vector<AbstractScope*> scopes;

	std::vector<bool> need_process;

	std::vector<std::vector<FoldNetwork*>> inner_input_networks;
	std::vector<std::vector<int>> inner_input_sizes;
	std::vector<double> scope_average_mod;
	std::vector<double> scope_scale_mod;
	// scope end average adjusted by score network

	std::vector<bool> is_branch;
	std::vector<Branch*> branches;

	std::vector<FoldNetwork*> score_networks;

	std::vector<int> compress_sizes;
	std::vector<bool> active_compress;
	std::vector<FoldNetwork*> compress_networks;

	int explore_index_inclusive;	// -1 if not on path
	int explore_type;
	int explore_end_non_inclusive;
	Fold* explore_fold;

};

#endif /* SCOPE_H */