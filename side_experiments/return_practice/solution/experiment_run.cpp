#include "experiment_run.h"

#include "abstract_node.h"
#include "experiment.h"

using namespace std;

ExperimentRun::~ExperimentRun() {
	for (map<Experiment*, ExperimentHistory*>::iterator it = this->experiment_histories.begin();
			it != this->experiment_histories.end(); it++) {
		delete it->second;
	}

	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
