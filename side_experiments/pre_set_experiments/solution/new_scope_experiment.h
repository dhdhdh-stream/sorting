#ifndef NEW_SCOPE_EXPERIMENT_H
#define NEW_SCOPE_EXPERIMENT_H

#include <utility>
#include <vector>

#include "abstract_experiment.h"
#include "context_layer.h"
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
	int sub_state_iter;

	Scope* new_scope;

	AbstractNode* exit_next_node;

	double score;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<int> verify_scope_history_sizes;
	#endif /* MDEBUG */

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch);
	~NewScopeExperiment();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void test_activate(AbstractNode*& curr_node,
					   Problem* problem,
					   std::vector<ContextLayer>& context,
					   RunHelper& run_helper,
					   NewScopeExperimentHistory* history);
	void test_backprop(double target_val,
					   RunHelper& run_helper);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper,
								 NewScopeExperimentHistory* history);
	void capture_verify_backprop();
	#endif /* MDEBUG */

	void finalize(Solution* duplicate);
};

class NewScopeExperimentHistory : public AbstractExperimentHistory {
public:
	int instance_count;

	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */