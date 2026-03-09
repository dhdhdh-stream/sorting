/**
 * - 60000 iters per ExploreExperiment
 * - 40000 iters per initial measure of EvalExperiment
 */

#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

#include <utility>
#include <vector>

class AbstractNode;
class Scope;
class SolutionWrapper;

const int EXPERIMENT_TYPE_EXPLORE = 0;
const int EXPERIMENT_TYPE_EVAL = 1;

class AbstractExperiment {
public:
	int type;

	virtual ~AbstractExperiment() {};

	virtual void check_activate(AbstractNode* experiment_node,
								std::vector<double>& obs,
								SolutionWrapper* wrapper) = 0;
	virtual void experiment_step(std::vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper) = 0;
	virtual void set_action(int action,
							SolutionWrapper* wrapper) = 0;
	virtual void experiment_exit_step(SolutionWrapper* wrapper) = 0;
};

class AbstractExperimentState {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentState() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */