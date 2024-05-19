#include "eval_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

EvalPassThroughExperiment::EvalPassThroughExperiment(
		Eval* eval_context) {
	this->type = EXPERIMENT_TYPE_EVAL_PASS_THROUGH;

	this->eval_context = eval_context;
	this->scope_context = this->eval_context->subscope;

	this->score_network = NULL;
	this->vs_network = NULL;

	this->ending_node = NULL;

	this->target_val_histories.reserve(NUM_DATAPOINTS);

	this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

EvalPassThroughExperiment::~EvalPassThroughExperiment() {
	cout << "outer delete " << this << endl;

	if (this->score_network != NULL) {
		delete this->score_network;
	}
	if (this->vs_network != NULL) {
		delete this->vs_network;
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

	for (int h_index = 0; h_index < (int)this->start_scope_histories.size(); h_index++) {
		delete this->start_scope_histories[h_index];
	}
	for (int h_index = 0; h_index < (int)this->end_scope_histories.size(); h_index++) {
		delete this->end_scope_histories[h_index];
	}
}

void EvalPassThroughExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

EvalPassThroughExperimentHistory::EvalPassThroughExperimentHistory(
		EvalPassThroughExperiment* experiment) {
	this->experiment = experiment;
}
