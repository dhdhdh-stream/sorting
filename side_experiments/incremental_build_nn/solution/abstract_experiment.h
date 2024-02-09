#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

const int EXPERIMENT_TYPE_OUTER = 0;
const int EXPERIMENT_TYPE_BRANCH = 1;
const int EXPERIMENT_TYPE_PASS_THROUGH = 2;
const int EXPERIMENT_TYPE_RETRAIN_BRANCH = 3;

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