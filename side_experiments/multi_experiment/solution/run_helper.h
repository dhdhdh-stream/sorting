#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <vector>

class AbstractExperimentHistory;

class RunHelper {
public:
	int num_actions;

	int num_experiments_seen;

	std::vector<AbstractExperimentHistory*> experiment_histories;

	bool early_exit;

	RunHelper();
	~RunHelper();
};

#endif /* RUN_HELPER_H */