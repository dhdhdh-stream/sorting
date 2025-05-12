#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <utility>
#include <vector>

#include "run_helper.h"

class AbstractNode;
class ObsNode;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

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
	ObsNode* node_context;

	int result;

	double improvement;

	virtual ~AbstractExperiment() {};
	virtual void decrement(ObsNode* experiment_node) = 0;

	virtual void activate(ObsNode* experiment_node,
						  AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history) = 0;
	virtual void backprop(double target_val,
						  RunHelper& run_helper) = 0;

	virtual void clean() = 0;
	virtual void add() = 0;
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */