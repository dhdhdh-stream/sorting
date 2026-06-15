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

ForceExperimentHistory::ForceExperimentHistory(ForceExperiment* experiment) {
	this->hit_branch = false;
}

ForceExperimentState::ForceExperimentState(ForceExperiment* experiment) {
	this->experiment = experiment;
}
