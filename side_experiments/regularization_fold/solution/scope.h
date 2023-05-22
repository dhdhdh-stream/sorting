#ifndef SCOPE_H
#define SCOPE_H

class Scope {
public:
	int id;

	int num_inputs;
	std::vector<ObjectDefinition*> input_types;

	// loop stuff

	std::vector<AbstractNode*> nodes;



	void activate(std::vector<double>& flat_vals,
				  std::vector<Object*>& inputs,
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