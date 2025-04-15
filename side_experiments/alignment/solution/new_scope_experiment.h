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

const int NEW_SCOPE_EXPERIMENT_STATE_MEASURE = 0;
const int NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST = 1;
const int NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND = 2;
#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY = 3;
#endif /* MDEBUG */

class NewScopeExperimentHistory;
class NewScopeExperiment : public AbstractExperiment {
public:
	int state;
	int state_iter;

	Scope* new_scope;

	double sum_score;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch,
					   AbstractNode* exit_next_node);
	~NewScopeExperiment();
	void decrement(AbstractNode* experiment_node);

	void activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
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
	void capture_verify_activate(AbstractNode*& curr_node,
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
	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */