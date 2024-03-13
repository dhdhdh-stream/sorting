#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include "run_helper.h"
#include "context_layer.h"

class AbstractNode;
class PassThroughExperiment;
class Problem;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_FAIL = 1;
const int EXPERIMENT_RESULT_SUCCESS = 2;

class AbstractExperimentHistory;
class AbstractExperiment {
public:
	int type;

	std::vector<Scope*> scope_context;
	std::vector<AbstractNode*> node_context;
	bool is_fuzzy_match;
	bool is_branch;
	int throw_id;

	PassThroughExperiment* parent_experiment;
	PassThroughExperiment* root_experiment;

	double average_remaining_experiments_from_start;
	double average_instances_per_run;

	int result;

	virtual ~AbstractExperiment() {};

	virtual bool activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  RunHelper& run_helper) = 0;

	virtual void backprop(double target_val,
						  RunHelper& run_helper) = 0;

	virtual void finalize() = 0;
	/**
	 * - also set to NULL in parent
	 */
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */