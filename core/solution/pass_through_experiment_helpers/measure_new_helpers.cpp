#include "pass_through_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::measure_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (this->best_step_types.size() == 0) {
		curr_node = this->best_exit_next_node;
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else {
			curr_node = this->best_scopes[0];
		}
	}
}

void PassThroughExperiment::measure_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if ((int)this->o_target_val_histories.size() >= NUM_DATAPOINTS) {
		#if defined(MDEBUG) && MDEBUG
		this->o_target_val_histories.clear();

		if (rand()%2 == 0) {
		#else
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		double new_average_score = sum_scores / NUM_DATAPOINTS;

		this->o_target_val_histories.clear();

		if (new_average_score >= this->existing_average_score) {
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_1ST_NUM_DATAPOINTS);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
