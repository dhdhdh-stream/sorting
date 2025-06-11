/**
 * - only 1 experiment per run
 *   - random experiments too likely to be catastrophic
 *     - such that main focus for other experiments will too likely end up being minimizing damage
 *       - rather than improvements to actual solution
 */

#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <utility>
#include <vector>

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;
const int EXPERIMENT_TYPE_NEW_SCOPE = 2;
const int EXPERIMENT_TYPE_COMMIT = 3;

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_FAIL = 1;
const int EXPERIMENT_RESULT_SUCCESS = 2;

class AbstractExperimentHistory;
class AbstractExperiment {
public:
	int type;

	Scope* scope_context;
	AbstractNode* node_context;
	bool is_branch;

	int result;

	double improvement;

	virtual ~AbstractExperiment() {};
	virtual void decrement(AbstractNode* experiment_node) = 0;

	virtual void check_activate(AbstractNode* experiment_node,
								bool is_branch,
								SolutionWrapper* wrapper) = 0;
	virtual void experiment_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper) = 0;
	virtual void set_action(int action,
							SolutionWrapper* wrapper) = 0;
	virtual void experiment_exit_step(SolutionWrapper* wrapper) = 0;
	virtual void backprop(double target_val,
						  SolutionWrapper* wrapper) = 0;

	virtual void clean() = 0;
	virtual void add(SolutionWrapper* wrapper) = 0;
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentHistory() {};
};

class AbstractExperimentState {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentState() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */