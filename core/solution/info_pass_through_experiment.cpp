#include "info_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "network.h"
#include "problem.h"
#include "scope.h"

using namespace std;

InfoPassThroughExperiment::InfoPassThroughExperiment(
		AbstractScope* scope_context,
		AbstractNode* node_context,
		int score_type) {
	this->type = EXPERIMENT_TYPE_INFO_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->score_type = score_type;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->ending_node = NULL;

	this->target_val_histories.reserve(NUM_DATAPOINTS);

	this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

InfoPassThroughExperiment::~InfoPassThroughExperiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
	}
	if (this->new_network != NULL) {
		delete this->new_network;
	}

	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		if (this->actions[s_index] != NULL) {
			delete this->actions[s_index];
		}
	}

	if (this->ending_node != NULL) {
		delete this->ending_node;
	}

	for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
		delete this->scope_histories[h_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void InfoPassThroughExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

InfoPassThroughExperimentHistory::InfoPassThroughExperimentHistory(
		InfoPassThroughExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;
}
