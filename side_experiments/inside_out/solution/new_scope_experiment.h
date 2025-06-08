#ifndef NEW_SCOPE_EXPERIMENT_H
#define NEW_SCOPE_EXPERIMENT_H

#include <utility>
#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeNode;
class Solution;
class SolutionWrapper;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 10;
const int NEW_SCOPE_NUM_LOCATIONS = 2;
#else
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 200;
const int NEW_SCOPE_NUM_LOCATIONS = 2;
#endif /* MDEBUG */

const int LOCATION_STATE_MEASURE_EXISTING = 0;
const int LOCATION_STATE_MEASURE_NEW = 1;
const int LOCATION_STATE_VERIFY_EXISTING_1ST = 2;
const int LOCATION_STATE_VERIFY_NEW_1ST = 3;
const int LOCATION_STATE_VERIFY_EXISTING_2ND = 4;
const int LOCATION_STATE_VERIFY_NEW_2ND = 5;

class NewScopeExperimentHistory;
class NewScopeExperiment : public AbstractExperiment {
public:
	int generalize_iter;

	Scope* new_scope;

	AbstractNode* test_location_start;
	bool test_location_is_branch;
	AbstractNode* test_location_exit;
	int test_location_state;
	double test_location_existing_score;
	double test_location_new_score;
	int test_location_count;

	std::vector<AbstractNode*> successful_location_starts;
	std::vector<bool> successful_location_is_branch;
	std::vector<ScopeNode*> successful_scope_nodes;

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch);
	~NewScopeExperiment();
	void decrement(AbstractNode* experiment_node);

	void pre_activate(SolutionWrapper* wrapper);
	void check_activate(AbstractNode* experiment_node,
						bool is_branch,
						SolutionWrapper* wrapper);
	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);
	void back_activate(SolutionWrapper* wrapper);
	void backprop(double target_val,
				  SolutionWrapper* wrapper);

	void test_backprop(double target_val,
					   NewScopeExperimentHistory* history);

	void clean();
	void add(SolutionWrapper* wrapper);
};

class NewScopeExperimentHistory : public AbstractExperimentHistory {
public:
	bool hit_test;

	int instance_count;
	AbstractNode* potential_start;
	bool potential_is_branch;

	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

class NewScopeExperimentState : public AbstractExperimentState {
public:
	NewScopeExperimentState(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */