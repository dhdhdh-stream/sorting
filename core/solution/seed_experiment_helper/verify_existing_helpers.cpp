#include "seed_experiment.h"

#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void SeedExperiment::verify_existing_activate(
		vector<ContextLayer>& context,
		SeedExperimentHistory* history) {
	switch (this->score_type) {
	case SCORE_TYPE_TRUTH:
		history->predicted_scores.push_back(vector<double>());
		break;
	case SCORE_TYPE_ALL:
		history->predicted_scores.push_back(vector<double>(context.size()-1, 0.0));
		for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
			ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
			Scope* scope = (Scope*)scope_history->scope;

			if (scope->eval_network != NULL) {
				scope_history->callback_experiment_history = history;
				scope_history->callback_experiment_indexes.push_back(
					(int)history->predicted_scores.size()-1);
				scope_history->callback_experiment_layers.push_back(l_index);
			}
		}
		break;
	}
}

void SeedExperiment::verify_existing_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	SeedExperimentHistory* history = (SeedExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history) / (int)scope_history->callback_experiment_indexes.size();
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void SeedExperiment::verify_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	SeedExperimentHistory* history = (SeedExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		double final_score;
		switch (this->score_type) {
		case SCORE_TYPE_TRUTH:
			final_score = (target_val - solution->average_score) / (int)history->predicted_scores.size();
			break;
		case SCORE_TYPE_ALL:
			{
				double sum_score = (target_val - solution->average_score) / (int)history->predicted_scores.size();
				for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
					sum_score += history->predicted_scores[i_index][l_index];
				}
				final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
			}
			break;
		}

		this->target_val_histories.push_back(final_score);
	}

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / num_instances;

		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = SEED_EXPERIMENT_STATE_VERIFY;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
