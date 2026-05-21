/**
 * - ActionNode always followed by ObsNode
 * - BranchNodes always preceded by ObsNode
 */

#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include "experiment_run.h"
#include "predict_run.h"

class Experiment;

const int NODE_TYPE_OBS = 0;
const int NODE_TYPE_ACTION = 1;
const int NODE_TYPE_BRANCH = 2;

class AbstractNode {
public:
	int type;

	int id;

	virtual ~AbstractNode() {};

	virtual void experiment_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 ExperimentRun& run) = 0;
	virtual void predict_step(PredictRun& run) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */