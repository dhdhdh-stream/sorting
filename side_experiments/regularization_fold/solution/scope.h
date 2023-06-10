#ifndef SCOPE_H
#define SCOPE_H

const int SCOPE_TYPE_FULL = 0;
const int SCOPE_TYPE_FETCH = 1;

class Scope {
public:
	int id;

	int num_states;
	std::vector<StateDefinition*> state_minimum_types;
	int starting_num_states;	// i.e., states that are necessary
	std::vector<bool> initialized_locally;
	std::vector<std::map<StateDefinition*, std::vector<int>>> state_dependencies;

	// loop stuff

	std::vector<AbstractNode*> nodes;

	int type;

	// for creating fetch
	std::vector<int> parent_scope_ids;
	std::vector<int> parent_node_ids;
	// TODO: for last layer, run partially, and if there's a branch, randomly choose down 1 path

	// TODO: add branch end nodes
	// - add previous nodes

	// TODO: add fetch node (instead of squashing fetch into new scope)

	// TODO: for early exit, add information on branch node, and don't branch if needed for fetch

	void activate(std::vector<double>& flat_vals,
				  std::vector<State>& input_vals,
				  double& predicted_score,
				  double& scale_factor,
				  std::vector<int>& scope_context,
				  std::vector<int>& node_context,
				  std::vector<ScopeHistory*>& context_histories,
				  int& early_exit_depth,
				  int& early_exit_node_id,
				  FoldHistory*& early_exit_fold_history,
				  int& explore_exit_depth,
				  int& explore_exit_node_id,
				  FoldHistory*& explore_exit_fold_history,
				  RunHelper& run_helper,
				  ScopeHistory* history);

	void fetch_activate(Fetch* fetch,
						int fetch_layer,
						);
};

class ScopeHistory {
public:
	Scope* scope;

	std::vector<std::vector<AbstractNodeHistory*>> node_histories;

	
};

#endif /* SCOPE_H */