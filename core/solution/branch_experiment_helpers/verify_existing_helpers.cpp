#include "branch_experiment.h"

#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void BranchExperiment::verify_existing_backprop(
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	this->scope_context->eval->activate(problem,
										run_helper,
										eval_history->end_scope_history);
	double predicted_impact = this->scope_context->eval->calc_vs(
		run_helper,
		eval_history);
	this->target_val_histories.push_back(predicted_impact);

	if (this->state == BRANCH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& (int)this->target_val_histories.size() >= VERIFY_1ST_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->verify_existing_average_score = sum_scores / VERIFY_1ST_NUM_DATAPOINTS;

		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = BRANCH_EXPERIMENT_STATE_VERIFY_1ST;
		this->state_iter = 0;
	} else if ((int)this->target_val_histories.size() >= VERIFY_2ND_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->verify_existing_average_score = sum_scores / VERIFY_2ND_NUM_DATAPOINTS;

		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = BRANCH_EXPERIMENT_STATE_VERIFY_2ND;
		this->state_iter = 0;
	}
}
