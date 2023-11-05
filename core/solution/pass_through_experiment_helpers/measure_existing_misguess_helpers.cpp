#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::measure_existing_misguess_activate(
		vector<ContextLayer>& context) {
	context[context.size() - this->scope_context.size()]
		.scope_history->inner_pass_through_experiment = this;
}

void PassThroughExperiment::measure_existing_misguess_parent_scope_end_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	double predicted_score = this->existing_average_score;

	for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
			it != context.back().input_state_vals.end(); it++) {
		map<int, double>::iterator weight_it = this->existing_input_state_weights.find(it->first);
		if (weight_it != this->existing_input_state_weights.end()) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				predicted_score += weight_it->second * normalized;
			} else {
				predicted_score += weight_it->second * it->second.val;
			}
		}
	}

	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		map<int, double>::iterator weight_it = this->existing_local_state_weights.find(it->first);
		if (weight_it != this->existing_local_state_weights.end()) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				predicted_score += weight_it->second * normalized;
			} else {
				predicted_score += weight_it->second * it->second.val;
			}
		}
	}

	for (map<State*, StateStatus>::iterator it = context.back().temp_state_vals.begin();
			it != context.back().temp_state_vals.end(); it++) {
		map<State*, double>::iterator weight_it = this->existing_temp_state_weights.find(it->first);
		if (weight_it != this->existing_temp_state_weights.end()) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				predicted_score += weight_it->second * normalized;
			} else {
				predicted_score += weight_it->second * it->second.val;
			}
		}
	}

	PassThroughExperimentOverallHistory* history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
	history->predicted_scores.push_back(predicted_score);
}

void PassThroughExperiment::measure_existing_misguess_backprop(
		double target_val,
		PassThroughExperimentOverallHistory* history) {
	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		this->i_misguess_histories.push_back((history->predicted_scores[i_index] - target_val) * (history->predicted_scores[i_index] - target_val));
	}

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		int num_instances = (int)this->i_misguess_histories.size();

		double sum_misguess = 0.0;
		for (int i_index = 0; i_index < (int)this->i_misguess_histories.size(); i_index++) {
			sum_misguess += i_misguess_histories[i_index];
		}
		this->existing_average_misguess = sum_misguess / num_instances;

		double sum_misguess_variance = 0.0;
		for (int i_index = 0; i_index < (int)this->i_misguess_histories.size(); i_index++) {
			sum_misguess_variance += (this->i_misguess_histories[i_index] - this->existing_average_misguess) * (this->i_misguess_histories[i_index] - this->existing_average_misguess);
		}
		this->existing_misguess_variance = sum_misguess_variance / num_instances;

		this->i_misguess_histories.clear();

		// reserve at least solution->curr_num_datapoints
		this->i_scope_histories.reserve(solution->curr_num_datapoints);
		this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
		this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
		this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
		this->i_target_val_histories.reserve(solution->curr_num_datapoints);

		this->state = PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW_MISGUESS;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
