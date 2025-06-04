#ifndef NEW_SCOPE_EXPERIMENT_H
#define NEW_SCOPE_EXPERIMENT_H

#include <utility>
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
const int NEW_SCOPE_NUM_LOCATIONS = 2;
#endif /* MDEBUG */

const int LOCATION_STATE_MEASURE_1_PERCENT = 0;
const int LOCATION_STATE_MEASURE_5_PERCENT = 1;
const int LOCATION_STATE_MEASURE_10_PERCENT = 2;
const int LOCATION_STATE_MEASURE_25_PERCENT = 3;
const int LOCATION_STATE_MEASURE_50_PERCENT = 4;

class NewScopeExperimentHistory;
class NewScopeExperiment : public AbstractExperiment {
public:
	int generalize_iter;

	Scope* new_scope;

	AbstractNode* test_location_start;
	bool test_location_is_branch;
	AbstractNode* test_location_exit;
	int test_location_state;
	double test_location_existing_sum_score;
	int test_location_existing_count;
	double test_location_new_sum_score;
	int test_location_new_count;

	std::vector<AbstractNode*> successful_location_starts;
	std::vector<bool> successful_location_is_branch;
	std::vector<ScopeNode*> successful_scope_nodes;

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch);
	~NewScopeExperiment();
	void decrement(AbstractNode* experiment_node);

	void pre_activate(RunHelper& run_helper);
	void activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void back_activate(RunHelper& run_helper,
					   ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_factor(Scope* scope,
						int original_node_id,
						int original_factor_index,
						int new_node_id,
						int new_factor_index);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);
	void replace_scope(Scope* original_scope,
					   Scope* new_scope,
					   int new_scope_node_id);

	void clean();
	void add();
};

class NewScopeExperimentHistory : public AbstractExperimentHistory {
public:
	bool hit_test;

	std::vector<int> local_successful_hits;

	int instance_count;
	AbstractNode* potential_start;
	bool potential_is_branch;

	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */