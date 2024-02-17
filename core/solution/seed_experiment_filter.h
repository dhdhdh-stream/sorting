#ifndef SEED_EXPERIMENT_FILTER_H
#define SEED_EXPERIMENT_FILTER_H

class SeedExperimentFilter : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;

	SeedExperiment* parent;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;

	double average_confidence;
	std::vector<double> linear_weights;
	std::vector<std::vector<int>> network_input_indexes;
	Network* network;

	/**
	 * - for filter
	 */
	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<ScopeNode*> existing_scopes;
	std::vector<ScopeNode*> potential_scopes;
	ExitNode* exit_node;

	AbstractNode* end_node;

	SeedExperimentFilter(SeedExperiment* parent,
						 std::vector<Scope*> scope_context,
						 std::vector<AbstractNode*> node_context,
						 bool is_branch,
						 std::vector<std::vector<Scope*>> input_scope_contexts,
						 std::vector<std::vector<AbstractNode*>> input_node_contexts,
						 double average_confidence,
						 std::vector<double> linear_weights,
						 std::vector<std::vector<int>> network_input_indexes,
						 Network* network,
						 std::vector<int> step_types,
						 std::vector<ActionNode*> actions,
						 std::vector<ScopeNode*> existing_scopes,
						 std::vector<ScopeNode*> potential_scopes,
						 int exit_depth,
						 AbstractNode* exit_node);
	~SeedExperimentFilter();

	void activate();

	void finalize_success();
	void clean_fail();
};

#endif /* SEED_EXPERIMENT_FILTER_H */