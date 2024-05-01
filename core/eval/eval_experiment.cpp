#include "eval_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "network.h"
#include "scope.h"

using namespace std;

EvalExperiment::EvalExperiment(AbstractNode* node_context,
							   bool is_branch) {
	this->type = EXPERIMENT_TYPE_EVAL;

	this->node_context = node_context;
	this->is_branch = is_branch;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->existing_decision_scope_histories.reserve(NUM_DATAPOINTS);
	this->existing_final_scope_histories.reserve(NUM_DATAPOINTS);
	this->existing_target_val_histories.reserve(NUM_DATAPOINTS);

	this->state = EVAL_EXPERIMENT_STATE_CAPTURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

EvalExperiment::~EvalExperiment() {
	for (int h_index = 0; h_index < (int)this->existing_decision_scope_histories.size(); h_index++) {
		delete this->existing_decision_scope_histories[h_index];
	}

	for (int h_index = 0; h_index < (int)this->existing_final_scope_histories.size(); h_index++) {
		delete this->existing_final_scope_histories[h_index];
	}

	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		delete this->actions[a_index];
	}

	for (int h_index = 0; h_index < (int)this->new_decision_scope_histories.size(); h_index++) {
		delete this->new_decision_scope_histories[h_index];
	}

	for (int h_index = 0; h_index < (int)this->new_final_scope_histories.size(); h_index++) {
		delete this->new_final_scope_histories[h_index];
	}

	if (this->eval_network != NULL) {
		delete this->eval_network;
	}

	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}
}

EvalExperimentHistory::EvalExperimentHistory(EvalExperiment* experiment) {
	this->experiment = experiment;
}
