#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include "action.h"
#include "run_helper.h"

class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_NEW_SCOPE = 1;

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

	virtual ~AbstractExperiment() {};
	virtual void decrement(AbstractNode* experiment_node) = 0;

	virtual void activate(AbstractNode* experiment_node,
						  bool is_branch,
						  AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history) = 0;
	virtual void update(AbstractExperimentHistory* history,
						double target_val) = 0;

	virtual void finalize() = 0;
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */