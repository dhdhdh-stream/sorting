#ifndef PREDICT_RUN_H
#define PREDICT_RUN_H

#include <vector>

class AbstractNode;
class Experiment;
class ExperimentHistory;
class WorldModelWrapper;

class PredictRun {
public:
	WorldModelWrapper* wrapper;

	AbstractNode* node_context;
	ExperimentState* experiment_context;

	std::vector<double> state;

	std::map<Experiment*, ExperimentHistory*> experiment_histories;
};

#endif /* PREDICT_RUN_H */