#include "experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"

using namespace std;

Experiment::Experiment() {
	this->curr_ramp = 0;
	this->measure_status = MEASURE_STATUS_N_A;

	this->existing_sum_scores = 0.0;
	this->existing_count = 0;
	this->new_sum_scores = 0.0;
	this->new_count = 0;

	this->state = EXPERIMENT_STATE_RAMP;
	this->state_iter = 0;
}

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
