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

	this->num_original_actions = 0;
	this->num_multi_instances = 0;
}

RunHelper::~RunHelper() {
	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}

	for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->multi_experiment_histories.begin();
			it != this->multi_experiment_histories.end(); it++) {
		delete it->second;
	}
}
