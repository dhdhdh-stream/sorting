#ifndef NEW_SCOPE_EXPERIMENT_H
#define NEW_SCOPE_EXPERIMENT_H

#include <map>
#include <vector>

#include "abstract_experiment.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 10;
const int NEW_SCOPE_NUM_LOCATIONS = 2;
#else
const int NEW_SCOPE_NUM_GENERALIZE_TRIES = 200;
const int NEW_SCOPE_NUM_LOCATIONS = 3;
#endif /* MDEBUG */

class NewScopeExperimentOverallHistory;
class NewScopeExperiment : public AbstractExperiment {
public:
	int generalize_iter;

	Scope* new_scope;

	std::vector<AbstractNode*> test_starts;
	std::vector<bool> test_is_branch;
	std::vector<AbstractNode*> test_exits;
	std::vector<double> test_existing_scores;
	std::vector<int> test_existing_counts;
	std::vector<double> test_new_scores;
	std::vector<int> test_new_counts;

	std::vector<AbstractNode*> successful_starts;
	std::vector<bool> successful_is_branch;
	std::vector<AbstractNode*> successful_exits;

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
	void backprop(AbstractExperimentInstanceHistory* instance_history,
				  double target_val);
	void update(AbstractExperimentOverallHistory* overall_history,
				double target_val);

	void test_activate(int test_index,
					   AbstractNode*& curr_node,
					   Problem* problem,
					   RunHelper& run_helper,
					   NewScopeExperimentOverallHistory* overall_history);
	void test_update(NewScopeExperimentOverallHistory* overall_history,
					 double target_val);

	void finalize();
};

class NewScopeExperimentOverallHistory : public AbstractExperimentOverallHistory {
public:
	std::map<int, bool> test_is_new;

	NewScopeExperimentOverallHistory(NewScopeExperiment* experiment);
};

#endif /* NEW_SCOPE_EXPERIMENT_H */