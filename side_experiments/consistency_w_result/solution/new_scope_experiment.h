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

const int LOCATION_STATE_MEASURE = 0;
const int LOCATION_STATE_VERIFY_1ST = 1;
const int LOCATION_STATE_VERIFY_2ND = 2;

class NewScopeExperimentHistory;
class NewScopeExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;
	int generalize_iter;

	Scope* new_scope;

	AbstractNode* test_location_start;
	bool test_location_is_branch;
	AbstractNode* test_location_exit;
	int test_location_state;
	double test_location_score;
	int test_location_count;

	std::vector<AbstractNode*> successful_location_starts;
	std::vector<bool> successful_location_is_branch;
	std::vector<ScopeNode*> successful_scope_nodes;

	std::vector<double> match_histories;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch);
	~NewScopeExperiment();
	void decrement(AbstractNode* experiment_node);

	void pre_activate(RunHelper& run_helper);
	void result_activate(AbstractNode* experiment_node,
						 bool is_branch,
						 AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper,
						 ScopeHistory* scope_history);
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

	void test_activate(AbstractNode*& curr_node,
					   Problem* problem,
					   RunHelper& run_helper,
					   ScopeHistory* scope_history,
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
	AbstractNode* potential_start;
	bool potential_is_branch;

	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */