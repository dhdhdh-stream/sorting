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

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 10;
const int NEW_SCOPE_MIN_NUM_LOCATIONS = 2;
#else
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 500;
const int NEW_SCOPE_MIN_NUM_LOCATIONS = 3;
#endif /* MDEBUG */

const int NEW_SCOPE_FOCUS_ITERS = 3;

class NewScopeExperimentHistory;
class NewScopeExperimentState;
class NewScopeExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	Scope* new_scope;

	AbstractNode* exit_next_node;

	double new_sum_scores;

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch,
					   Scope* new_scope);
	~NewScopeExperiment();

	void result_check_activate(AbstractNode* experiment_node,
							   bool is_branch,
							   SolutionWrapper* wrapper);

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

class NewScopeOverallExperiment {
public:
	Scope* scope_context;

	Scope* new_scope;

	int generalize_iter;

	NewScopeExperiment* curr_experiment;
	std::vector<NewScopeExperiment*> successful_experiments;

	std::vector<ScopeHistory*> new_scope_histories;
	std::vector<double> new_target_val_histories;

	NewScopeOverallExperiment(Scope* new_scope,
							  Scope* scope_context);
	~NewScopeOverallExperiment();

	double calc_impact();

	void clean();
	void add(SolutionWrapper* wrapper);
};

#endif /* NEW_SCOPE_EXPERIMENT_H */