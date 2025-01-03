#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <vector>

#include "run_helper.h"
#include "context_layer.h"

class AbstractNode;
class BranchNode;
class Problem;
class Scope;
class Solution;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;
const int EXPERIMENT_TYPE_NEW_ACTION = 2;

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

	double average_remaining_experiments_from_start;

	int result;

	virtual ~AbstractExperiment() {};
	virtual void decrement(AbstractNode* experiment_node) = 0;

	virtual bool result_activate(AbstractNode* experiment_node,
								 bool is_branch,
								 AbstractNode*& curr_node,
								 Problem* problem,
								 std::vector<ContextLayer>& context,
								 RunHelper& run_helper) = 0;
	virtual bool activate(AbstractNode* experiment_node,
						  bool is_branch,
						  AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper) = 0;
	virtual void backprop(double target_val,
						  RunHelper& run_helper) = 0;

	virtual void finalize(Solution* duplicate) = 0;
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */