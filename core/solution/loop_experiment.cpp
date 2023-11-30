#include "loop_experiment.h"

#include "globals.h"
#include "potential_scope_node.h"
#include "solution.h"

using namespace std;

LoopExperiment::LoopExperiment(vector<int> scope_context,
							   vector<int> node_context,
							   PotentialScopeNode* potential_loop) {
	this->type = EXPERIMENT_TYPE_LOOP;

	this->scope_context = scope_context;
	this->node_context = node_context;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->state = LOOP_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->potential_loop = potential_loop;

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);
	this->i_scope_histories.reserve(solution->curr_num_datapoints);
	this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
	this->i_target_val_histories.reserve(solution->curr_num_datapoints);

	this->measure_score = 0.0;
	this->measure_num_instances = 0;
	this->measure_sum_iters = 0;
}

LoopExperiment::~LoopExperiment() {
	if (this->potential_loop != NULL) {
		delete this->potential_loop;
	}
}

LoopExperimentInstanceHistory::LoopExperimentInstanceHistory(LoopExperiment* experiment) {
	this->experiment = experiment;
}

LoopExperimentInstanceHistory::LoopExperimentInstanceHistory(LoopExperimentInstanceHistory* original) {
	this->experiment = original->experiment;

	for (int i_index = 0; i_index < (int)original->iter_histories.size(); i_index++) {
		this->iter_histories.push_back(new PotentialScopeNodeHistory(original->iter_histories[i_index]));
	}
}

LoopExperimentInstanceHistory::~LoopExperimentInstanceHistory() {
	for (int i_index = 0; i_index < (int)this->iter_histories.size(); i_index++) {
		delete this->iter_histories[i_index];
	}
}

LoopExperimentOverallHistory::LoopExperimentOverallHistory(LoopExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
}
