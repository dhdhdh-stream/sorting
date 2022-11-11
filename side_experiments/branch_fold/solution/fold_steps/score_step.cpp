#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::score_step(vector<vector<double>>& flat_vals,
					  vector<vector<vector<double>>>& inner_flat_vals,
					  double target_val) {
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

	vector<double> obs_input;
	if (this->compound_actions[this->nodes.size()] == NULL) {
		obs_input = flat_vals[this->nodes.size()];
	} else {
		vector<double> action_input;
		if (this->action->num_inputs > 0) {
			for (int i_index = 0; i_index < (int)this->action_input_input_networks.size(); i_index++) {
				this->action_input_input_networks[i_index]->activate(state_vals[this->action_input_input_layer[i_index]],
																	 s_input_vals[this->action_input_input_layer[i_index]]);
				for (int s_index = 0; s_index < this->action_input_input_sizes[i_index]; s_index++) {
					s_input_vals[this->action_input_input_layer[i_index]+1].push_back(
						this->action_input_input_networks[i_index]->output->acti_vals[s_index]);
				}
			}

			this->small_action_input_network->activate(state_vals.back(),
													   s_input_vals.back());
			action_input.reserve(this->action->num_inputs);
			for (int i_index = 0; i_index < this->action->num_inputs; i_index++) {
				action_input.push_back(this->small_action_input_network->output->acti_vals[i_index]);
			}
		}
		double scope_predicted_score = 0.0;
		this->action->activate(inner_flat_vals[this->nodes.size()],
							   action_input,
							   scope_predicted_score);
		obs_input = this->action->outputs;
		obs_input.push_back(scope_predicted_score);
	}

	this->obs_network->activate(obs_input);
	state_vals.push_back(vector<double>(this->new_layer_size));
	s_input_vals.push_back(vector<double>());
	for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
		state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
	}

	this->test_score_network->activate(state_vals,
									   s_input_vals);
	predicted_score += this->test_score_network->output->acti_vals[0];

	this->sum_error += (target_val - predicted_score)*(target_val - predicted_score);

	if (this->stage == STAGE_LEARN) {
		if ((this->stage_iter+1)%10000 == 0) {
			cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		vector<double> score_errors{target_val - predicted_score};
		// if (this->stage_iter <= 120000) {
		if (this->stage_iter <= 240000) {
			this->test_score_network->backprop_weights_with_no_error_signal(
				score_errors,
				0.05);
		// } else if (this->stage_iter <= 160000) {
		} else if (this->stage_iter <= 270000) {
			this->test_score_network->backprop_weights_with_no_error_signal(
				score_errors,
				0.01);
		} else {
			this->test_score_network->backprop_weights_with_no_error_signal(
				score_errors,
				0.002);
		}
	}
}
