#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int id;

	int num_inputs;
	std::vector<StateDefinition*> input_types;
	std::vector<ScopeObjectDefinition*> input_objects;

	int num_local;
	std::vector<StateDefinition*> local_types;
	std::vector<ScopeObjectDefinition*> local_objects;

	// loop stuff

	std::vector<AbstractNode*> nodes;

	// TODO: find a way to mark where object has been tested but unneeded

	// TODO: when determining what type scope input is, look at what state is needed, and select most general type that fits

	void activate(std::vector<double>& flat_vals,
				  std::vector<double>& input_vals,
				  std::vector<bool>& inputs_initialized,
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