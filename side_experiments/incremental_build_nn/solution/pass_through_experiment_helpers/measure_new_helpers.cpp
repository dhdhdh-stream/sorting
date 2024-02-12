#include "pass_through_experiment.h"

#include <cmath>

#include "action_node.h"
#include "constants.h"
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
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
			delete action_node_history;
		} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_existing_scopes[s_index]);
			this->best_existing_scopes[s_index]->potential_activate(
				problem,
				context,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_potential_scopes[s_index]);
			this->best_potential_scopes[s_index]->potential_activate(
				problem,
				context,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node = this->best_exit_node;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node = this->best_exit_node;
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
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		if (score_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;
			this->state_iter = 0;
		#if defined(MDEBUG) && MDEBUG
		} else if (rand()%2 == 0) {
		#else
		} else if (new_average_score >= this->existing_average_score) {
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
