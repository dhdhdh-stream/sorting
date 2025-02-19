#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;

class RunHelper {
public:
	int num_actions;

	std::map<AbstractExperiment*, AbstractExperimentHistory*> experiment_histories;

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */