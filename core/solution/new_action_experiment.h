// /**
//  * - don't search for branch possibilities
//  *   - expensive and count on new action capturing needed branches
//  */

// #ifndef NEW_ACTION_EXPERIMENT_H
// #define NEW_ACTION_EXPERIMENT_H

// #include <set>
// #include <vector>

// #include "abstract_experiment.h"
// #include "context_layer.h"
// #include "run_helper.h"

// class AbstractNode;
// class Problem;
// class Scope;
// class ScopeHistory;
// class Solution;

// /**
//  * - randomly create candidate at start
//  */
// const int NEW_ACTION_EXPERIMENT_MEASURE_EXISTING = 0;
// const int NEW_ACTION_EXPERIMENT_MEASURE_NEW = 1;
// /**
//  * - don't worry about verifying
//  */

// #if defined(MDEBUG) && MDEBUG
// const int NEW_ACTION_NUM_GENERALIZE_TRIES = 10;
// const int NEW_ACTION_MIN_LOCATIONS = 2;
// #else
// const int NEW_ACTION_NUM_GENERALIZE_TRIES = 100;
// const int NEW_ACTION_MIN_LOCATIONS = 4;
// #endif /* MDEBUG */

// class NewActionExperimentHistory;
// class NewActionExperiment : public AbstractExperiment {
// public:
// 	int generalize_iter;

// 	AbstractNode* starting_node;
// 	std::set<AbstractNode*> included_nodes;

// 	std::vector<AbstractNode*> test_location_starts;
// 	std::vector<bool> test_location_is_branch;
// 	std::vector<AbstractNode*> test_location_exits;
// 	std::vector<int> test_location_states;
// 	std::vector<int> test_location_state_iters;
// 	std::vector<double> test_location_existing_scores;
// 	std::vector<int> test_location_existing_counts;
// 	std::vector<double> test_location_new_scores;
// 	std::vector<int> test_location_new_counts;

// 	/**
// 	 * - add when experimenting and pop if fail
// 	 * 
// 	 * - when deleting, remove locations 1-by-1 and fully delete when empty
// 	 */
// 	std::vector<AbstractNode*> successful_location_starts;
// 	std::vector<bool> successful_location_is_branch;
// 	std::vector<AbstractNode*> successful_location_exits;

// 	NewActionExperiment(Scope* scope_context,
// 						AbstractNode* node_context,
// 						bool is_branch);
// 	void decrement(AbstractNode* experiment_node);

// 	bool activate(AbstractNode* experiment_node,
// 				  bool is_branch,
// 				  AbstractNode*& curr_node,
// 				  Problem* problem,
// 				  std::vector<ContextLayer>& context,
// 				  RunHelper& run_helper);
// 	void back_activate(ScopeHistory* scope_history,
// 					   NewActionExperimentHistory* history);
// 	void backprop(double target_val,
// 				  RunHelper& run_helper);

// 	void test_activate(int location_index,
// 					   AbstractNode*& curr_node,
// 					   Problem* problem,
// 					   std::vector<ContextLayer>& context,
// 					   RunHelper& run_helper,
// 					   NewActionExperimentHistory* history);
// 	void test_backprop(double target_val,
// 					   RunHelper& run_helper);

// 	void successful_activate(int location_index,
// 							 AbstractNode*& curr_node,
// 							 Problem* problem,
// 							 std::vector<ContextLayer>& context,
// 							 RunHelper& run_helper,
// 							 NewActionExperimentHistory* history);

// 	void add_new_test_location(ScopeHistory* scope_history);

// 	void finalize(Solution* duplicate);
// };

// class NewActionExperimentHistory : public AbstractExperimentHistory {
// public:
// 	int test_location_index;

// 	NewActionExperimentHistory(NewActionExperiment* experiment);
// };

// #endif /* NEW_ACTION_EXPERIMENT_H */