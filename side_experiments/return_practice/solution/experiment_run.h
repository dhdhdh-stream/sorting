#ifndef EXPERIMENT_RUN_H
#define EXPERIMENT_RUN_H

#include <map>
#include <vector>

class AbstractNode;
class AbstractNodeHistory;
class Experiment;
class ExperimentHistory;
class ExperimentState;
class Wrapper;

class ExperimentRun {
public:
	Wrapper* wrapper;

	AbstractNode* node_context;
	ExperimentState* experiment_context;

	std::vector<double> state;

	std::map<Experiment*, ExperimentHistory*> experiment_histories;

	std::map<int, AbstractNodeHistory*> node_histories;

	std::vector<std::vector<double>> obs_histories;
	std::vector<int> action_histories;

	~ExperimentRun();
};

#endif /* EXPERIMENT_RUN_H */