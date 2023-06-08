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
};

#endif /* SCOPE_H */