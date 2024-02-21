#ifndef SEED_EXPERIMENT_GATHER_H
#define SEED_EXPERIMENT_GATHER_H

class SeedExperimentGather : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;
	bool is_branch;

	SeedExperiment* parent;

	/**
	 * - add temporarily to local scope
	 */
	std::vector<int> step_types;
	std::vector<ActionNode*> actions;
	std::vector<ScopeNode*> existing_scopes;
	std::vector<ScopeNode*> potential_scopes;
	int exit_depth;
	AbstractNode* exit_next_node;
	ExitNode* exit_node;

	bool is_candidate;

	SeedExperimentGather(SeedExperiment* parent,
						 std::vector<Scope*> scope_context,
						 std::vector<AbstractNode*> node_context,
						 bool is_branch,
						 std::vector<int> step_types,
						 std::vector<ActionNode*> actions,
						 std::vector<ScopeNode*> existing_scopes,
						 std::vector<ScopeNode*> potential_scopes,
						 int exit_depth,
						 AbstractNode* exit_node);
	~SeedExperimentGather();

	void activate(AbstractNode*& curr_node,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void add_to_scope();

	void finalize_success();
	void clean_fail();
};

#endif /* SEED_EXPERIMENT_GATHER_H */