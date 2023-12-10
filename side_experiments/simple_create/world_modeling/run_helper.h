#ifndef RUN_HELPER_H
#define RUN_HELPER_H

#include <set>
#include <vector>

class Experiment;

const int RUN_TYPE_EXPLORE = 0;
const int RUN_TYPE_UPDATE = 1;

class RunHelper {
public:
	int type;

	std::set<Experiment*> experiments_seen;
	std::vector<Experiment*> experiments_seen_order;

	Experiment* selected_experiment;

	RunHelper() {
		this->selected_experiment = NULL;
	}
};

#endif /* RUN_HELPER_H */