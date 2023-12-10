#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <set>
#include <vector>

class AbstractExperiment;

const int RUN_TYPE_EXPLORE = 0;
const int RUN_TYPE_UPDATE = 1;

class RunHelper {
public:
	int type;

	std::set<AbstractExperiment*> experiments_seen;
	std::vector<AbstractExperiment*> experiments_seen_order;

	AbstractExperiment* selected_experiment;

	RunHelper() {
		this->selected_experiment = NULL;
	}
};

#endif /* RUN_HELPER_H */