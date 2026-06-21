#include "experiment_run.h"

#include "abstract_node.h"
#include "compare_experiment.h"
#include "force_experiment.h"
#include "state_network.h"

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

	for (int s_index = 0; s_index < (int)this->obs_network_histories.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->obs_network_histories[s_index].size(); n_index++) {
			delete this->obs_network_histories[s_index][n_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->action_network_histories.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->action_network_histories[s_index].size(); n_index++) {
			delete this->action_network_histories[s_index][n_index];
		}
	}

	if (this->compare_experiment_history != NULL) {
		delete this->compare_experiment_history;
	}
}
