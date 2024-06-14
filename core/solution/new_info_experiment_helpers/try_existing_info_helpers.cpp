#include "new_info_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_scope.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EXISTING_INFO_NUM_DATAPOINTS = 5;
const int EXISTING_INFO_TRUTH_NUM_DATAPOINTS = 2;
const int EXISTING_INFO_VERIFY_NUM_DATAPOINTS = 10;
const int EXISTING_INFO_VERIFY_TRUTH_NUM_DATAPOINTS = 2;
#else
const int EXISTING_INFO_NUM_DATAPOINTS = 200;
const int EXISTING_INFO_TRUTH_NUM_DATAPOINTS = 10;
const int EXISTING_INFO_VERIFY_NUM_DATAPOINTS = 2000;
const int EXISTING_INFO_VERIFY_TRUTH_NUM_DATAPOINTS = 100;
#endif /* MDEBUG */

bool NewInfoExperiment::try_existing_info_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewInfoExperimentHistory* history) {
	bool inner_is_positive;
	solution->info_scopes[this->existing_info_scope_index]->activate(
		problem,
		run_helper,
		inner_is_positive);

	history->predicted_scores.push_back(vector<double>(context.size(), 0.0));
	for (int l_index = 0; l_index < (int)context.size(); l_index++) {
		if (context[l_index].scope->eval_network != NULL) {
			context[l_index].scope_history->callback_experiment_history = history;
			context[l_index].scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			context[l_index].scope_history->callback_experiment_layers.push_back(l_index);
		}
	}

	if ((this->existing_is_negate && !inner_is_positive)
			|| (!this->existing_is_negate && inner_is_positive)) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		return true;
	} else {
		return false;
	}
}

void NewInfoExperiment::try_existing_info_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(context.back().scope_history);
	}
	for (int i_index = 0; i_index < (int)context.back().scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[context.back().scope_history->callback_experiment_indexes[i_index]]
			[context.back().scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void NewInfoExperiment::try_existing_info_backprop(double target_val,
												   RunHelper& run_helper) {
	bool is_fail = false;

	if (run_helper.exceeded_limit) {
		is_fail = true;
	} else {
		NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

		this->state_iter++;
		if (this->state_iter == EXISTING_INFO_TRUTH_NUM_DATAPOINTS
				&& this->sub_state_iter >= EXISTING_INFO_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (false) {
			#else
			double average_score = this->combined_score / this->sub_state_iter;
			if (average_score < this->existing_average_score) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->state_iter == EXISTING_INFO_VERIFY_TRUTH_NUM_DATAPOINTS
				&& this->sub_state_iter >= EXISTING_INFO_VERIFY_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%((int)solution->info_scopes.size()*2) == 0) {
			#else
			double average_score = this->combined_score / this->sub_state_iter;
			if (average_score < this->existing_average_score) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		}

		for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
			double sum_score = 0.0;
			for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
				sum_score += history->predicted_scores[i_index][l_index];
			}
			double final_score = (sum_score / (int)history->predicted_scores[i_index].size() + target_val - solution->average_score) / 2.0;
			this->combined_score += final_score;
			this->sub_state_iter++;

			if (this->sub_state_iter == EXISTING_INFO_NUM_DATAPOINTS
					&& this->state_iter >= EXISTING_INFO_TRUTH_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				double average_score = this->combined_score / this->sub_state_iter;
				if (average_score < this->existing_average_score) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if (this->sub_state_iter == EXISTING_INFO_VERIFY_NUM_DATAPOINTS
					&& this->state_iter >= EXISTING_INFO_VERIFY_TRUTH_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%((int)solution->info_scopes.size()*2) == 0) {
				#else
				double average_score = this->combined_score / this->sub_state_iter;
				if (average_score < this->existing_average_score) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			}
		}
	}

	if (is_fail) {
		if (this->existing_is_negate == false) {
			this->existing_is_negate = true;
		} else {
			this->existing_info_scope_index++;
			this->existing_is_negate = false;
		}
		if (this->existing_info_scope_index >= (int)solution->info_scopes.size()) {
			this->use_existing = false;

			this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

			this->state = NEW_INFO_EXPERIMENT_STATE_VERIFY_EXISTING;
			this->state_iter = 0;
		} else {
			this->combined_score = 0.0;

			this->state_iter = 0;
			this->sub_state_iter = 0;
		}
	} else if (this->sub_state_iter >= EXISTING_INFO_VERIFY_NUM_DATAPOINTS
			&& this->state_iter >= EXISTING_INFO_VERIFY_TRUTH_NUM_DATAPOINTS) {
		this->use_existing = true;

		this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

		this->state = NEW_INFO_EXPERIMENT_STATE_VERIFY_EXISTING;
		this->state_iter = 0;
	}
}
