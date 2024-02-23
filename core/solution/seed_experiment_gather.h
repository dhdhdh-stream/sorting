#ifndef SEED_EXPERIMENT_GATHER_H
#define SEED_EXPERIMENT_GATHER_H

#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
#include "run_helper.h"

class AbstractNode;
class ActionNode;
class ExitNode;
class Scope;
class ScopeNode;
class SeedExperiment;

class SeedExperimentGather : public AbstractExperiment {
public:
	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;
	bool is_branch;

	SeedExperiment* parent;

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

	bool activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  int& exit_depth,
				  AbstractNode*& exit_node,
				  RunHelper& run_helper,
				  AbstractExperimentHistory*& history);

	void add_to_scope();

	void finalize();

	// unused
	void backprop(double target_val,
				  RunHelper& run_helper,
				  AbstractExperimentHistory* history);
};

#endif /* SEED_EXPERIMENT_GATHER_H */