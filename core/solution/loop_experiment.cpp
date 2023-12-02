#include "loop_experiment.h"

#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "solution.h"
#include "state.h"

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

	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		delete this->new_states[s_index];
	}

	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
		delete this->i_scope_histories[h_index];
	}
}

LoopExperimentInstanceHistory::LoopExperimentInstanceHistory(LoopExperiment* experiment) {
	this->experiment = experiment;
}

LoopExperimentInstanceHistory::LoopExperimentInstanceHistory(LoopExperimentInstanceHistory* original) {
	this->experiment = original->experiment;

	this->potential_loop_history = new PotentialScopeNodeHistory(original->potential_loop_history);
}

LoopExperimentInstanceHistory::~LoopExperimentInstanceHistory() {
	delete this->potential_loop_history;
}

LoopExperimentOverallHistory::LoopExperimentOverallHistory(LoopExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
}
