#include "experiment_run.h"

#include "abstract_node.h"
#include "experiment.h"
#include "force_experiment.h"

using namespace std;

ExperimentRun::ExperimentRun() {
	this->force_experiment_history = NULL;
}

ExperimentRun::~ExperimentRun() {
	for (map<Experiment*, ExperimentHistory*>::iterator it = this->experiment_histories.begin();
			it != this->experiment_histories.end(); it++) {
		delete it->second;
	}

	if (this->force_experiment_history != NULL) {
		delete this->force_experiment_history;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
