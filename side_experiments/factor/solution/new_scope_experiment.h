#ifndef NEW_SCOPE_EXPERIMENT_H
#define NEW_SCOPE_EXPERIMENT_H

#include <utility>
#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class AbstractNode;
class ObsNode;
class Problem;
class Scope;
class Solution;

const int NEW_SCOPE_EXPERIMENT_STATE_EXPLORE = 0;
#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY = 1;
#endif /* MDEBUG */

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
	int state;
	int state_iter;
	int generalize_iter;

	Scope* new_scope;

	ObsNode* test_location_start;
	AbstractNode* test_location_exit;
	int test_location_state;
	double test_location_existing_score;
	double test_location_new_score;
	int test_location_count;

	std::vector<ObsNode*> successful_location_starts;
	std::vector<ObsNode*> successful_obs_nodes;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	NewScopeExperiment(Scope* scope_context,
					   ObsNode* node_context);
	~NewScopeExperiment();
	void decrement(ObsNode* experiment_node);

	void pre_activate(RunHelper& run_helper);
	void activate(ObsNode* experiment_node,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void back_activate(RunHelper& run_helper,
					   ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void test_activate(AbstractNode*& curr_node,
					   Problem* problem,
					   RunHelper& run_helper,
					   NewScopeExperimentHistory* history);
	void test_backprop(double target_val,
					   RunHelper& run_helper,
					   NewScopeExperimentHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(int location_index,
								 AbstractNode*& curr_node,
								 Problem* problem,
								 RunHelper& run_helper,
								 ScopeHistory* scope_history);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void clean();
	void add();
};

class NewScopeExperimentHistory : public AbstractExperimentHistory {
public:
	bool hit_test;

	int instance_count;
	ObsNode* potential_start;

	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */