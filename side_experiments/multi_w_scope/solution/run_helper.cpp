#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "solution.h"

using namespace std;

RunHelper::RunHelper() {
	this->num_actions = 0;

	this->num_original_actions = 0;
	this->num_experiment_instances = 0;

	this->has_explore = false;
}

RunHelper::~RunHelper() {
	for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories.begin();
			it != this->experiment_histories.end(); it++) {
		delete it->second;
	}
}
