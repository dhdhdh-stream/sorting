#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <set>
#include <vector>

class AbstractExperiment;
class AbstractExperimentHistory;

class RunHelper {
public:
	int num_actions;

	std::set<AbstractExperiment*> experiments_seen;

	std::vector<AbstractExperimentHistory*> experiment_histories;

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */