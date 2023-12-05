#include "outer_experiment.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void OuterExperiment::measure_existing_score_activate(
		Problem& problem,
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
							 0,
							 root_history);

	delete root_history;
}

void OuterExperiment::measure_existing_score_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->target_val_histories.push_back(target_val);

	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;

			if (solution->max_depth < 50) {
				solution->depth_limit = solution->max_depth + 10;
			} else {
				solution->depth_limit = (int)(1.2*(double)solution->max_depth);
			}
		}
	}

	if ((int)this->target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / solution->curr_num_datapoints;

		// cout << "Outer" << endl;
		// cout << "this->existing_average_score: " << this->existing_average_score << endl;
		// cout << endl;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->target_val_histories[d_index] - this->existing_average_score) * (this->target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / solution->curr_num_datapoints;

		this->target_val_histories.clear();

		this->state = OUTER_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
