#ifndef NEW_SCOPE_EXPERIMENT_H
#define NEW_SCOPE_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Scope;
class SolutionWrapper;

const int NEW_SCOPE_EXPERIMENT_STATE_C1 = 0;
const int NEW_SCOPE_EXPERIMENT_STATE_C2 = 1;
const int NEW_SCOPE_EXPERIMENT_STATE_C3 = 2;
const int NEW_SCOPE_EXPERIMENT_STATE_C4 = 3;

class NewScopeExperimentHistory;
class NewScopeExperimentState;
class NewScopeExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	Scope* new_scope;
	bool is_new;

	AbstractNode* exit_next_node;

	double new_sum_scores;

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch,
					   Scope* new_scope,
					   bool is_new);
	~NewScopeExperiment();

	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
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
				  SolutionWrapper* wrapper);

	void clean();
	void add(SolutionWrapper* wrapper);
};

class NewScopeExperimentHistory : public AbstractExperimentHistory {
public:
	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

class NewScopeExperimentState : public AbstractExperimentState {
public:
	NewScopeExperimentState(NewScopeExperiment* experiment);
};

#endif /* NEW_SCOPE_EXPERIMENT_H */