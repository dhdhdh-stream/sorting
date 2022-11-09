#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::compress_input_step(vector<vector<double>>& flat_vals,
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
		this->action->activate(inner_flat_vals[this->nodes.size()],
							   action_input,
							   predicted_score);
		obs_input = this->action->outputs;
	}

	this->obs_network->activate(obs_input);
	state_vals.push_back(vector<double>(this->new_layer_size));
	s_input_vals.push_back(vector<double>());
	for (int s_index = 0; s_index < this->new_layer_size; s_index++) {
		state_vals.back()[s_index] = this->obs_network->output->acti_vals[s_index];
	}

	for (int i_index = 0; i_index < (int)this->score_input_networks.size(); i_index++) {
		this->score_input_networks[i_index]->activate(state_vals[this->score_input_layer[i_index]],
													  s_input_vals[this->score_input_layer[i_index]]);
		for (int s_index = 0; s_index < this->score_input_sizes[i_index]; s_index++) {
			s_input_vals[this->score_input_layer[i_index]+1].push_back(
				this->score_input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	this->small_score_network->activate(state_vals.back(),
										s_input_vals.back());
	predicted_score += this->small_score_network->output->acti_vals[0];

	// this->compress_num_layers > 0
	// this->compress_new_size > 0
	for (int i_index = 0; i_index < (int)this->input_networks.size(); i_index++) {
		this->input_networks[i_index]->activate(state_vals[this->input_layer[i_index]],
												s_input_vals[this->input_layer[i_index]]);
		for (int s_index = 0; s_index < this->input_sizes[i_index]; s_index++) {
			s_input_vals[this->input_layer[i_index]+1].push_back(
				this->input_networks[i_index]->output->acti_vals[s_index]);
		}
	}

	this->curr_compression_network->activate_fold(state_vals,
												  s_input_vals);

	this->test_compression_network->activate_fold(state_vals,
												  s_input_vals);

	vector<double> compression_errors(this->compress_new_size);
	for (int s_index = 0; s_index < this->compress_new_size; s_index++) {
		compression_errors[s_index] = this->curr_compression_network->output->acti_vals[s_index]
			- this->test_compression_network->output->acti_vals[s_index];
		this->sum_error += compression_errors[s_index]*compression_errors[s_index];
	}

	if (this->stage == STAGE_LEARN) {
		if ((this->stage_iter+1)%10000 == 0) {
			cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
			this->sum_error = 0.0;
		}

		if (this->input_networks.size() == 0) {
			if (this->stage_iter <= 160000) {
				this->test_compression_network->backprop_weights_with_no_error_signal(
					compression_errors,
					0.01);
			} else {
				this->test_compression_network->backprop_weights_with_no_error_signal(
					compression_errors,
					0.002);
			}
		} else {
			if (this->stage_iter <= 160000) {
				this->test_compression_network->backprop_new_s_input(
					this->input_layer.back()+1,
					this->input_sizes.back(),
					compression_errors,
					0.01);
			} else {
				this->test_compression_network->backprop_new_s_input(
					this->input_layer.back()+1,
					this->input_sizes.back(),
					compression_errors,
					0.002);
			}
			vector<double> input_errors;
			input_errors.reserve(this->input_sizes.back());
			int layer_size = (int)this->test_compression_network->s_input_inputs[this->input_layer.back()+1]->errors.size();
			for (int st_index = layer_size-this->input_sizes.back(); st_index < layer_size; st_index++) {
				input_errors.push_back(this->test_compression_network->s_input_inputs[this->input_layer.back()+1]->errors[st_index]);
				this->test_compression_network->s_input_inputs[this->input_layer.back()+1]->errors[st_index] = 0.0;
			}
			if (this->stage_iter <= 80000) {
				this->input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.05);
			} else if (this->stage_iter <= 160000) {
				this->input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.01);
			} else {
				this->input_networks.back()->backprop_weights_with_no_error_signal(input_errors, 0.002);
			}
		}
	}
}
