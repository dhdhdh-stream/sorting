#include "retrain_branch_experiment.h"

#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

RetrainBranchExperiment::RetrainBranchExperiment(BranchNode* branch_node) {
	this->type = EXPERIMENT_TYPE_RETRAIN_BRANCH;

	this->branch_node = branch_node;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->input_scope_contexts = this->branch_node->input_scope_contexts;
	this->input_node_contexts = this->branch_node->input_node_contexts;

	this->original_network_input_indexes = this->branch_node->original_network_input_indexes;
	if (this->branch_node->original_network == NULL) {
		this->original_network = NULL;
	} else {
		this->original_network = new Network(this->branch_node->original_network);
	}
	this->branch_network_input_indexes = this->branch_node->branch_network_input_indexes;
	if (this->branch_node->branch_network == NULL) {
		this->branch_network = NULL;
	} else {
		this->branch_network = new Network(this->branch_node->branch_network);
	}

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);

	this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->combined_score = 0.0;

	this->result = EXPERIMENT_RESULT_NA;
}

RetrainBranchExperiment::~RetrainBranchExperiment() {
	if (this->original_network != NULL) {
		delete this->original_network;
	}
	if (this->branch_network != NULL) {
		delete this->branch_network;
	}

	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
		delete this->i_scope_histories[h_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

RetrainBranchExperimentOverallHistory::RetrainBranchExperimentOverallHistory(
		RetrainBranchExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
}
