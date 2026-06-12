#include "experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "wrapper.h"

using namespace std;

Experiment::~Experiment() {
	// temp
	cout << "delete experiment" << endl;

	if (this->original_network != NULL) {
		delete this->original_network;
	}
	if (this->branch_network != NULL) {
		delete this->branch_network;
	}
}

void Experiment::pad_new_state(int num_add) {
	this->original_network->add_inputs(num_add);
	this->branch_network->add_inputs(num_add);
}

ExperimentHistory::ExperimentHistory(Experiment* experiment) {
	uniform_int_distribution<int> on_distribution(0, EXPERIMENT_NUM_GEARS);
	if (experiment->curr_ramp >= on_distribution(generator)) {
		this->is_on = true;
	} else {
		this->is_on = false;
	}

	this->hit_branch = false;
}

ExperimentState::ExperimentState(Experiment* experiment) {
	this->experiment = experiment;
}
