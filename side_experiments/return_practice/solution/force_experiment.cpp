#include "force_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "state_network.h"
#include "wrapper.h"

using namespace std;

ForceExperiment::ForceExperiment() {
	this->type = EXPERIMENT_TYPE_FORCE;
}

ForceExperiment::~ForceExperiment() {
	if (this->original_network != NULL) {
		delete this->original_network;
	}
}

bool ForceExperiment::further_than(ForceExperiment* other) {
	if (this->state < other->state) {
		return false;
	} else if (this->state > other->state) {
		return true;
	} else {
		if (this->state_iter <= other->state_iter) {
			return false;
		} else {
			return true;
		}
	}
}

ForceExperimentHistory::ForceExperimentHistory(ForceExperiment* experiment) {
	this->experiment = experiment;
}

ForceExperimentState::ForceExperimentState(ForceExperiment* experiment) {
	this->experiment = experiment;
}
