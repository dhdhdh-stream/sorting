#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <vector>

#include "context_layer.h"
#include "problem.h"
#include "run_helper.h"

class AbstractNode;

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;

class AbstractExperimentHistory;
class AbstractExperiment {
public:
	int type;

	std::vector<int> scope_context;
	std::vector<int> node_context;

	double average_remaining_experiments_from_start;

	virtual ~AbstractExperiment() {};
	virtual void activate(AbstractNode*& curr_node,
						  Problem& problem,
						  std::vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  AbstractExperimentHistory*& history) = 0;
};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentHistory() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */