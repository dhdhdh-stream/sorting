#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include "run_helper.h"
#include "context_layer.h"

class AbstractNode;
class Problem;

const int EXPERIMENT_TYPE_OUTER = 0;
const int EXPERIMENT_TYPE_BRANCH = 1;
const int EXPERIMENT_TYPE_PASS_THROUGH = 2;
const int EXPERIMENT_TYPE_RETRAIN_BRANCH = 3;
const int EXPERIMENT_TYPE_SEED = 4;
const int EXPERIMENT_TYPE_SEED_FILTER = 5;
const int EXPERIMENT_TYPE_SEED_GATHER = 6;

const int EXPERIMENT_RESULT_NA = 0;
const int EXPERIMENT_RESULT_FAIL = 1;
const int EXPERIMENT_RESULT_SUCCESS = 2;

class AbstractExperimentHistory;
class AbstractExperiment {
public:
	int type;

	int result;

	double average_remaining_experiments_from_start;

	virtual ~AbstractExperiment() {};

	virtual bool activate(AbstractNode*& curr_node,
						  Problem* problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  AbstractExperimentHistory*& history) = 0;

	virtual void backprop(double target_val,
						  RunHelper& run_helper,
						  AbstractExperimentHistory* history) = 0;

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