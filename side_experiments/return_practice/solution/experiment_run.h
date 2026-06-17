#ifndef EXPERIMENT_RUN_H
#define EXPERIMENT_RUN_H

#include <map>
#include <vector>

class AbstractExperimentState;
class AbstractNode;
class AbstractNodeHistory;
class ForceExperiment;
class ForceExperimentHistory;
class Wrapper;

class ExperimentRun {
public:
	Wrapper* wrapper;

	AbstractNode* node_context;
	AbstractExperimentState* experiment_context;

	std::vector<double> state;

	bool should_force;
	std::map<ForceExperiment*, ForceExperimentHistory*> force_experiment_histories;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<std::vector<double>> obs_histories;
	std::vector<int> action_histories;

	~ExperimentRun();
};

#endif /* EXPERIMENT_RUN_H */