#ifndef PREDICT_RUN_H
#define PREDICT_RUN_H

#include <map>
#include <vector>

class AbstractNode;
class Experiment;
class ExperimentHistory;
class Wrapper;

class PredictRun {
public:
	Wrapper* wrapper;

	AbstractNode* node_context;

	std::vector<double> state;

	std::map<Experiment*, ExperimentHistory*> experiment_histories;

	~PredictRun();
};

#endif /* PREDICT_RUN_H */