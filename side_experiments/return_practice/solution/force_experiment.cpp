#include "force_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "state_network.h"
#include "wrapper.h"

using namespace std;

ForceExperiment::ForceExperiment(AbstractNode* node_context,
								 bool is_branch,
								 AbstractNode* exit_next_node,
								 Wrapper* wrapper) {
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->state = FORCE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
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
