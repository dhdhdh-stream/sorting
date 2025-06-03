#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "solution.h"

using namespace std;

RunHelper::RunHelper() {
	this->num_actions = 1;

	this->num_experiment_instances = 0;
}

RunHelper::~RunHelper() {
	for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories.begin();
			it != this->experiment_histories.end(); it++) {
		delete it->second;
	}
}
