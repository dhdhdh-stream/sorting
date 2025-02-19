#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <map>
#include <vector>

class AbstractExperiment;
class AbstractExperimentOverallHistory;
class AbstractExperimentInstanceHistory;

class RunHelper {
public:
	int num_actions;

	std::map<AbstractExperiment*, AbstractExperimentOverallHistory*> overall_histories;
	std::vector<AbstractExperimentInstanceHistory*> instance_histories;

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */