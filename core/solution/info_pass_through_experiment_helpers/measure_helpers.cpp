#include "info_pass_through_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void InfoPassThroughExperiment::measure_activate(
		AbstractNode*& curr_node) {
	if (this->actions.size() == 0) {
		curr_node = this->exit_next_node;
	} else {
		curr_node = this->actions[0];
	}
}

void InfoPassThroughExperiment::measure_info_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		bool& is_positive) {
	InfoPassThroughExperimentHistory* history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories.back();

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

	vector<double> new_input_vals(this->new_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->new_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.find(
			this->new_input_node_contexts[i_index]);
		if (it != context.back().scope_history->node_histories.end()) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
			new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
		}
	}
	this->new_network->activate(new_input_vals);
	#if defined(MDEBUG) && MDEBUG
	#else
	double new_predicted_score = this->new_network->output->acti_vals[0];
	#endif /* MDEBUG */

	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_positive = true;
	} else {
		is_positive = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (new_predicted_score >= 0.0) {
		is_positive = true;
	} else {
		is_positive = false;
	}
	#endif /* MDEBUG */
}

void InfoPassThroughExperiment::measure_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	InfoPassThroughExperimentHistory* history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories.back();

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

void InfoPassThroughExperiment::measure_backprop(
		double target_val,
		RunHelper& run_helper) {
	if (run_helper.exceeded_limit) {
		this->result = EXPERIMENT_RESULT_FAIL;
	} else {
		InfoPassThroughExperimentHistory* history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories.back();

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

			this->combined_score += final_score;
			this->sub_state_iter++;
		}

		this->state_iter++;
		if (this->sub_state_iter >= NUM_DATAPOINTS
				&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
			this->combined_score /= this->sub_state_iter;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->combined_score > this->existing_average_score) {
			#endif /* MDEBUG */
				this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

				this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
