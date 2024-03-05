#include "pass_through_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::measure_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	if (this->throw_id != -1) {
		run_helper.throw_id = -1;
	}

	if (this->best_step_types.size() == 0) {
		if (this->exit_node != NULL) {
			curr_node = this->exit_node;
		} else {
			curr_node = this->best_exit_next_node;
		}
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else if (this->best_step_types[0] == STEP_TYPE_EXISTING_SCOPE) {
			curr_node = this->best_existing_scopes[0];
		} else {
			curr_node = this->best_potential_scopes[0];
		}
	}
}

void PassThroughExperiment::measure_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		#if defined(MDEBUG) && MDEBUG
		this->o_target_val_histories.clear();

		if (rand()%4 == 0) {
		#else
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		double new_average_score = sum_scores / solution->curr_num_datapoints;

		this->o_target_val_histories.clear();

		double score_improvement = new_average_score - this->existing_average_score;
		double score_improvement_t_score = score_improvement
			/ (this->existing_score_standard_deviation / sqrt(solution->curr_num_datapoints));

		if (score_improvement_t_score > 1.960) {
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;
			this->state_iter = 0;
		#if defined(MDEBUG) && MDEBUG
		} else if (this->best_step_types.size() > 0
				&& rand()%2 == 0) {
		#else
		} else if (this->best_step_types.size() > 0
				&& score_improvement_t_score >= 0.0) {
		#endif /* MDEBUG */
			this->new_is_better = false;

			this->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
