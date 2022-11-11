#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::inner_scope_input_step(vector<vector<double>>& flat_vals,
								  vector<vector<vector<double>>>& inner_flat_vals,
								  double target_val) {
	if ((this->stage_iter+1)%10000 == 0) {
		cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
		this->sum_error = 0.0;
	}

	vector<vector<double>> state_vals;
	vector<vector<double>> s_input_vals;
	double predicted_score = this->average_score;

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->compound_actions[n_index] == NULL) {
			nodes[n_index]->activate(state_vals,
									 s_input_vals,
									 flat_vals[n_index],
									 predicted_score);
		} else {
			nodes[n_index]->activate(state_vals,
									 s_input_vals,
									 inner_flat_vals[n_index],
									 predicted_score);
		}
	}

	vector<vector<double>> input_fold_inputs(this->nodes.size());
	this->curr_scope_input_folds[this->nodes.size()]->activate(input_fold_inputs,
															   state_vals);

	this->curr_action_input_network->activate(state_vals,
											  s_input_vals);

	vector<double> action_input_errors(this->action->num_inputs);
	for (int i_index = 0; i_index < this->action->num_inputs; i_index++) {
		action_input_errors[i_index] = this->curr_scope_input_folds[this->nodes.size()]->output->acti_vals[i_index]
			- this->curr_action_input_network->output->acti_vals[i_index];
		this->sum_error += action_input_errors[i_index]*action_input_errors[i_index];
	}
	// if (this->stage_iter <= 120000) {
	if (this->stage_iter <= 240000) {
		this->curr_action_input_network->backprop_weights_with_no_error_signal(
			action_input_errors,
			0.05);
	// } else if (this->stage_iter <= 160000) {
	} else if (this->stage_iter <= 270000) {
		this->curr_action_input_network->backprop_weights_with_no_error_signal(
			action_input_errors,
			0.01);
	} else {
		this->curr_action_input_network->backprop_weights_with_no_error_signal(
			action_input_errors,
			0.002);
	}
}
