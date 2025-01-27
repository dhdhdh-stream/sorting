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

const int NEW_SCOPE_EXPERIMENT_STATE_EXPLORE = 0;
#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY = 1;
#endif /* MDEBUG */

const int NEW_SCOPE_EXPERIMENT_MEASURE = 0;
const int NEW_SCOPE_EXPERIMENT_VERIFY_1ST = 1;
const int NEW_SCOPE_EXPERIMENT_VERIFY_2ND = 2;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 10;
const int NEW_SCOPE_NUM_LOCATIONS = 2;
#else
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 200;
const int NEW_SCOPE_NUM_LOCATIONS = 3;
#endif /* MDEBUG */

const int NEW_SCOPE_EXPERIMENT_TYPE_IN_PLACE = 0;
const int NEW_SCOPE_EXPERIMENT_TYPE_ANY = 1;

class NewScopeExperimentHistory;
class NewScopeExperiment : public AbstractExperiment {
public:
	int new_scope_experiment_type;

	int state;
	int state_iter;
	int generalize_iter;

	Scope* new_scope;

	std::vector<AbstractNode*> test_location_starts;
	std::vector<bool> test_location_is_branch;
	std::vector<AbstractNode*> test_location_exits;
	std::vector<int> test_location_states;
	std::vector<double> test_location_scores;
	std::vector<int> test_location_counts;
	std::vector<int> test_location_truth_counts;
	std::vector<ScopeNode*> test_scope_nodes;

	std::vector<AbstractNode*> successful_location_starts;
	std::vector<bool> successful_location_is_branch;
	std::vector<ScopeNode*> successful_scope_nodes;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	std::vector<int> verify_scope_history_sizes;
	#endif /* MDEBUG */

	NewScopeExperiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch);
	~NewScopeExperiment();
	void decrement(AbstractNode* experiment_node);

	void pre_activate(std::vector<ContextLayer>& context,
					  RunHelper& run_helper);

	bool result_activate(AbstractNode* experiment_node,
						 bool is_branch,
						 AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);
	bool activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void back_activate(std::vector<ContextLayer>& context,
					   RunHelper& run_helper,
					   ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void test_activate(int location_index,
					   AbstractNode*& curr_node,
					   Problem* problem,
					   std::vector<ContextLayer>& context,
					   RunHelper& run_helper,
					   NewScopeExperimentHistory* history);
	void test_backprop(double target_val,
					   RunHelper& run_helper);

	void add_new_test_location(NewScopeExperimentHistory* history);

	#if defined(MDEBUG) && MDEBUG
	void capture_verify_activate(int location_index,
								 AbstractNode*& curr_node,
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
	int test_location_index;

	int instance_count;

	std::vector<AbstractNode*> selected_path_nodes;
	std::vector<bool> selected_path_is_branch;

	NewScopeExperimentHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_ACTION_EXPERIMENT_H */