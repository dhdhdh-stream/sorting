#include "orientation_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

OrientationExperiment::OrientationExperiment(Eval* eval_context,
											 AbstractNode* node_context,
											 bool is_branch) {
	this->type = EXPERIMENT_TYPE_ORIENTATION;

	this->eval_context = eval_context;
	this->scope_context = this->eval_context->subscope;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */

	this->existing_network = NULL;
	this->new_network = NULL;

	this->ending_node = NULL;

	this->scope_histories.reserve(NUM_DATAPOINTS);
	this->target_val_histories.reserve(NUM_DATAPOINTS);

	this->state = ORIENTATION_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

OrientationExperiment::~OrientationExperiment() {
	cout << "outer delete" << endl;

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

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		if (this->scopes[s_index] != NULL) {
			delete this->scopes[s_index];
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

void OrientationExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

OrientationExperimentHistory::OrientationExperimentHistory(OrientationExperiment* experiment) {
	this->experiment = experiment;
}
