#include "experiment_run.h"

#include "abstract_node.h"
#include "compare_experiment.h"
#include "force_experiment.h"

using namespace std;

ExperimentRun::ExperimentRun() {
	this->compare_experiment_history = NULL;
}

ExperimentRun::~ExperimentRun() {
	for (map<ForceExperiment*, ForceExperimentHistory*>::iterator it = this->force_experiment_histories.begin();
			it != this->force_experiment_histories.end(); it++) {
		delete it->second;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}

	if (this->compare_experiment_history != NULL) {
		delete this->compare_experiment_history;
	}
}
