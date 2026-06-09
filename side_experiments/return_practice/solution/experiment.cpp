#include "experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "wrapper.h"

using namespace std;

Experiment::Experiment(AbstractNode* node_context,
					   bool is_branch,
					   Wrapper* wrapper) {
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->original_network = NULL;
	this->branch_network = NULL;

	this->starting_iter = wrapper->iter;

	this->state = EXPERIMENT_STATE_GATHER;
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

void Experiment::pad_new_state(int num_add) {
	switch (this->state) {
	case EXPERIMENT_STATE_GATHER:
		this->start_state_history.clear();
		this->state_iter = 0;
		break;
	case EXPERIMENT_STATE_RAMP:
	case EXPERIMENT_STATE_MEASURE:
		this->original_network->add_inputs(num_add);
		this->branch_network->add_inputs(num_add);
		break;
	}
}

ExperimentHistory::ExperimentHistory(Experiment* experiment) {
	switch (experiment->state) {
	case EXPERIMENT_STATE_GATHER:
		this->is_on = false;
		break;
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
