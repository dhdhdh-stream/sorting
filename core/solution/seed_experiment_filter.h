#ifndef SEED_EXPERIMENT_FILTER_H
#define SEED_EXPERIMENT_FILTER_H

class SeedExperimentFilter : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;

	SeedExperiment* parent;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<AbstractNode*>> input_node_contexts;
	std::vector<std::vector<int>> network_input_indexes;
	Network* network;
	double average_misguess;
	double misguess_variance;

	AbstractNode* seed_next_node;
	std::vector<int> filter_step_types;
	std::vector<ActionNode*> filter_actions;
	std::vector<ScopeNode*> filter_existing_scopes;
	std::vector<ScopeNode*> filter_potential_scopes;
	int filter_exit_depth;
	AbstractNode* filter_exit_next_node;

	BranchNode* branch_node;
	ExitNode* filter_exit_node;

	bool is_candidate;

	SeedExperimentFilter(SeedExperiment* parent,
						 std::vector<Scope*> scope_context,
						 std::vector<AbstractNode*> node_context,
						 bool is_branch,
						 AbstractNode* seed_next_node,
						 std::vector<int> filter_step_types,
						 std::vector<ActionNode*> filter_actions,
						 std::vector<ScopeNode*> filter_existing_scopes,
						 std::vector<ScopeNode*> filter_potential_scopes,
						 int filter_exit_depth,
						 AbstractNode* filter_exit_next_node);
	~SeedExperimentFilter();

	void activate();

	void find_activate();



	void add_to_scope();



	void finalize_success();
	void clean_fail();
};

#endif /* SEED_EXPERIMENT_FILTER_H */