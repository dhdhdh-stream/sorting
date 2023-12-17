#include "retrain_branch_experiment.h"

#include "globals.h"
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

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);

	this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->combined_score = 0.0;
}

RetrainBranchExperiment::~RetrainBranchExperiment() {
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
