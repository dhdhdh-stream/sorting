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
		Eval* eval_context,
		AbstractNode* node_context,
		bool is_branch) {
	this->type = EXPERIMENT_TYPE_EVAL_PASS_THROUGH;

	this->eval_context = eval_context;
	this->scope_context = this->eval_context->subscope;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->network = NULL;

	this->ending_node = NULL;

	this->o_target_val_histories.reserve(NUM_DATAPOINTS);

	this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

EvalPassThroughExperiment::~EvalPassThroughExperiment() {
	cout << "outer delete " << this << endl;

	if (this->network != NULL) {
		delete this->network;
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

	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
		delete this->i_scope_histories[h_index];
	}
}

void EvalPassThroughExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

EvalPassThroughExperimentHistory::EvalPassThroughExperimentHistory(
		EvalPassThroughExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;
}
