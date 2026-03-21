#include "experiment.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"

using namespace std;

Experiment::Experiment(ObsNode* node_context) {
	this->type = EXPERIMENT_TYPE_EXPERIMENT;

	this->node_context = node_context;

	this->existing_network = NULL;
	this->existing_signal_network = NULL;

	this->state = EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
}

Experiment::~Experiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	if (this->existing_signal_network != NULL) {
		delete this->existing_signal_network;
	}

	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}

	for (int n_index = 0; n_index < (int)this->new_networks.size(); n_index++) {
		delete this->new_networks[n_index];
	}
}

ExperimentHistory::ExperimentHistory(Experiment* experiment) {
	switch (experiment->state) {
	case EXPERIMENT_STATE_RAMP:
	case EXPERIMENT_STATE_MEASURE:
		{
			uniform_int_distribution<int> on_distribution(0, EXPERIMENT_NUM_GEARS);
			if (experiment->curr_ramp >= on_distribution(generator)) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	}

	this->hit_branch = false;
}

ExperimentState::ExperimentState(Experiment* experiment) {
	this->experiment = experiment;
}
