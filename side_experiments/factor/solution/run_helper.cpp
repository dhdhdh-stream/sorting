#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "multi_pass_through_experiment.h"
#include "solution.h"

using namespace std;

RunHelper::RunHelper() {
	this->num_analyze = 0;
	this->num_actions = 0;

	this->experiment_history = NULL;

	this->has_explore = false;
}

RunHelper::~RunHelper() {
	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}

	for (map<MultiPassThroughExperiment*, MultiPassThroughExperimentHistory*>::iterator it = this->multi_experiment_histories.begin();
			it != this->multi_experiment_histories.end(); it++) {
		delete it->second;
	}
}
