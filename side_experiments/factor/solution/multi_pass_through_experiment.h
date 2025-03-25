/**
 * TODO: limit # of experiments if chance of being above average drops below 10%?
 */

#ifndef MULTI_PASS_THROUGH_EXPERIMENT_H
#define MULTI_PASS_THROUGH_EXPERIMENT_H

#include <vector>

#include "abstract_experiment.h"
#include "action.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class Solution;

class MultiPassThroughExperimentHistory;
class MultiPassThroughExperiment : public AbstractExperiment {
public:
	int id;

	AbstractNode* exit_next_node;

	std::vector<int> step_types;
	std::vector<Action> actions;
	std::vector<Scope*> scopes;

	std::vector<double> existing_target_vals;
	std::vector<std::vector<int>> existing_influence_indexes;
	std::vector<double> new_target_vals;
	std::vector<std::vector<int>> new_influence_indexes;
	std::map<int, int> influence_mapping;

	double improvement;

	std::vector<MultiPassThroughExperiment*> conflicts;
	std::vector<Scope*> conflicting_scopes;

	MultiPassThroughExperiment(Scope* scope_context,
							   AbstractNode* node_context,
							   bool is_branch);
	void decrement(AbstractNode* experiment_node);

	void activate(AbstractNode* experiment_node,
				  bool is_branch,
				  AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);
	void backprop(double target_val,
				  RunHelper& run_helper);

	void finalize(Solution* duplicate);
};

class MultiPassThroughExperimentHistory : public AbstractExperimentHistory {
public:
	bool is_active;

	MultiPassThroughExperimentHistory(MultiPassThroughExperiment* experiment);
};

#endif /* MULTI_PASS_THROUGH_EXPERIMENT_H */