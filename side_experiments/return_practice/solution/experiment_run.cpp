#include "experiment_run.h"

#include "abstract_node.h"
#include "force_experiment.h"

using namespace std;

ExperimentRun::~ExperimentRun() {
	for (map<ForceExperiment*, ForceExperimentHistory*>::iterator it = this->force_experiment_histories.begin();
			it != this->force_experiment_histories.end(); it++) {
		delete it->second;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
