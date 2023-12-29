#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

class AbstractNode;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;
const int EXPERIMENT_TYPE_RETRAIN_BRANCH = 2;
const int EXPERIMENT_TYPE_RETRAIN_LOOP = 3;

class AbstractExperimentHistory;
class AbstractExperiment {
public:
	int type;

	double average_remaining_experiments_from_start;

	virtual ~AbstractExperiment() {};
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */