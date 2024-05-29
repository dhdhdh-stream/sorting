#include "orientation_experiment.h"

#include "constants.h"
#include "eval.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void OrientationExperiment::verify_existing_backprop(
		EvalHistory* outer_eval_history,
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	double target_impact;
	if (context.size() == 1) {
		target_impact = problem->score_result(run_helper.num_decisions);
	} else {
		target_impact = context[context.size()-2].scope->eval->calc_impact(outer_eval_history);
	}

	double existing_predicted_impact = this->eval_context->calc_impact(eval_history);

	double existing_misguess = (target_impact - existing_predicted_impact) * (target_impact - existing_predicted_impact);
	this->target_val_histories.push_back(existing_misguess);

	if ((int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_NUM_DATAPOINTS; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / VERIFY_NUM_DATAPOINTS;

		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = ORIENTATION_EXPERIMENT_STATE_VERIFY;
		this->state_iter = 0;
	}
}
