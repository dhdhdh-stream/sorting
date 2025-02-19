#include "run_helper.h"

#include "abstract_experiment.h"

using namespace std;

RunHelper::RunHelper() {
	this->num_actions = 0;
}

RunHelper::~RunHelper() {
	for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = this->experiment_histories.begin();
			it != this->experiment_histories.end(); it++) {
		delete it->second;
	}
}
