#ifndef SCOPE_EXPERIMENT_H
#define SCOPE_EXPERIMENT_H

#include <set>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ObsNode;
class Scope;
class SolutionWrapper;

class ScopeExperimentHistory;
class ScopeExperimentState;
class ScopeExperiment : public AbstractExperiment {
public:
	ObsNode* node_context;
	Scope* new_scope;
	AbstractNode* exit_next_node;

	double sum_scores;
	int count;

	double total_sum_scores;
	int total_count;

	double local_improvement;
	double global_improvement;

	ScopeExperiment(ObsNode* node_context,
					Scope* new_scope,
					AbstractNode* exit_next_node);
	~ScopeExperiment();

	void check_activate(AbstractNode* experiment_node,
						std::vector<double>& obs,
						SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 bool& fetch_action,
						 SolutionWrapper* wrapper);
	void set_action(int action,
					SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper,
				  std::set<Scope*>& updated_scopes);

	void add(SolutionWrapper* wrapper);
};

class ScopeExperimentHistory {
public:
	bool is_hit;

	ScopeExperimentHistory(ScopeExperiment* experiment);
};

class ScopeExperimentState : public AbstractExperimentState {
public:
	ScopeExperimentState(ScopeExperiment* experiment);
};

#endif /* SCOPE_EXPERIMENT_H */