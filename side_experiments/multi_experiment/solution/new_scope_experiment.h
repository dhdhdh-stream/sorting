#ifndef NEW_SCOPE_EXPERIMENT_H
#define NEW_SCOPE_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeNode;
class Solution;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 10;
const int NEW_SCOPE_NUM_LOCATIONS = 2;
#else
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 200;
const int NEW_SCOPE_NUM_LOCATIONS = 3;
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

	std::vector<AbstractNode*> test_location_starts;
	std::vector<bool> test_location_is_branch;
	std::vector<AbstractNode*> test_location_exits;
	std::vector<int> test_location_states;
	std::vector<double> test_location_existing_scores;
	std::vector<int> test_location_existing_counts;
	std::vector<double> test_location_new_scores;
	std::vector<int> test_location_new_counts;

	std::vector<AbstractNode*> successful_location_starts;
	std::vector<bool> successful_location_is_branch;
	std::vector<AbstractNode*> successful_location_exits;

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch);
	~NewScopeExperiment();
	void decrement(AbstractNode* experiment_node);

	void activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void back_activate(RunHelper& run_helper,
					   ScopeHistory* scope_history);
	void backprop(AbstractExperimentHistory* history,
				  double target_val);
	void update();

	void test_backprop(NewScopeExperimentHistory* history,
					   double target_val);
	void test_update();

	void cleanup();
	void add();

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
};

class NewScopeExperimentHistory : public AbstractExperimentHistory {
public:
	int test_location_index;

	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */