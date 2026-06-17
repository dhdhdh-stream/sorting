#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

class ExperimentRun;
class PredictRun;

const int EXPERIMENT_TYPE_RAMP = 0;
const int EXPERIMENT_TYPE_FORCE = 1;

class AbstractExperiment {
public:
	int type;

	virtual ~AbstractExperiment() {};

	virtual void experiment_activate(ExperimentRun* run) = 0;
	virtual void experiment_step(int& action,
								 bool& is_next,
								 ExperimentRun* run) = 0;
};

class AbstractExperimentState {
public:
	AbstractExperiment* experiment;

	virtual ~AbstractExperimentState() {};
};

#endif /* ABSTRACT_EXPERIMENT_H */