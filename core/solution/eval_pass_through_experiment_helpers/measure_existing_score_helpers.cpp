#include "eval_pass_through_experiment.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void EvalPassThroughExperiment::measure_existing_score_backprop(
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	EvalPassThroughExperimentHistory* history = (EvalPassThroughExperimentHistory*)run_helper.experiment_scope_history->experiment_histories.back();

	double target_val;
	if (context.size() == 1) {
		double starting_score = 1.0;
		double ending_score = problem->score_result(run_helper.num_decisions);
		target_val = ending_score - starting_score;
	} else {
		context[context.size()-2].scope->eval->activate(
			problem,
			run_helper,
			history->outer_eval_history->end_scope_history);
		target_val = context[context.size()-2].scope->eval->calc_vs(
			run_helper,
			history->outer_eval_history);
	}

	this->target_val_histories.push_back(target_val);

	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / NUM_DATAPOINTS;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_score_variance += (this->target_val_histories[d_index] - this->existing_average_score) * (this->target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_standard_deviation = sqrt(sum_score_variance / NUM_DATAPOINTS);
		if (this->existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->target_val_histories.clear();

		this->score_misguess_histories.reserve(2 * NUM_DATAPOINTS);
		this->vs_misguess_histories.reserve(NUM_DATAPOINTS);

		this->state = EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING;
		this->state_iter = 0;
	}
}
