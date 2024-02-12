#include "outer_experiment.h"

#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void OuterExperiment::verify_existing_activate(
		Problem* problem,
		RunHelper& run_helper) {
	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = solution->root;
	context.back().node = NULL;

	ScopeHistory* root_history = new ScopeHistory(solution->root);
	context.back().scope_history = root_history;

	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	solution->root->activate(problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper,
							 root_history);

	delete root_history;
}

void OuterExperiment::verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->target_val_histories.push_back(target_val);

	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;
		}
	}

	if (this->state == OUTER_EXPERIMENT_STATE_VERIFY_1ST_EXISTING
			&& (int)this->target_val_histories.size() >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->target_val_histories[d_index] - this->existing_average_score) * (this->target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->target_val_histories.clear();

		this->target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->state = OUTER_EXPERIMENT_STATE_VERIFY_1ST_NEW;
		this->state_iter = 0;
	} else if ((int)this->target_val_histories.size() >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->target_val_histories[d_index] - this->existing_average_score) * (this->target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->target_val_histories.clear();

		this->target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->state = OUTER_EXPERIMENT_STATE_VERIFY_2ND_NEW;
		this->state_iter = 0;
	}
}
