#include "run_helper.h"

#include "abstract_experiment.h"

using namespace std;

RunHelper::RunHelper() {
	this->num_actions = 0;
}

RunHelper::~RunHelper() {
	for (map<AbstractExperiment*, AbstractExperimentOverallHistory*>::iterator it = this->overall_histories.begin();
			it != this->overall_histories.end(); it++) {
		delete it->second;
	}

	for (int h_index = 0; h_index < (int)this->instance_histories.size(); h_index++) {
		delete this->instance_histories[h_index];
	}
}
