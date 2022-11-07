#include "fold.h"

#include <cmath>
#include <iostream>

using namespace std;

void Fold::compress_small_step(vector<vector<double>>& flat_vals,
							   vector<vector<vector<double>>>& inner_flat_vals,
							   double target_val) {
	if ((this->stage_iter+1)%10000 == 0) {
		cout << this->stage_iter << " sum_error: " << this->sum_error << endl;
		this->sum_error = 0.0;
	}

	vector<vector<double>> fold_input;
	vector<vector<vector<double>>> input_fold_inputs(this->sequence_length);

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

		fold_input.push_back(vector<double>());	// empty
		for (int i_index = (int)this->nodes.size()+1; i_index < this->sequence_length; i_index++) {
			if (this->compound_actions[i_index] != NULL) {
				input_fold_inputs[i_index].push_back(vector<double>());	// empty
			}
		}
	}

	vector<double> obs_input;
	if (this->compound_actions[this->nodes.size()] == NULL) {
		obs_input = flat_vals[this->nodes.size()];
	} else {
		vector<double> action_input;
		if (this->action->num_inputs > 0) {
			this->action_input_network->activate(state_vals.back(),
												 s_input_vals.back());
			action_input.reserve(this->action->num_inputs);
			for (int i_index = 0; i_index < this->action->num_inputs; i_index++) {
				action_input.push_back(this->action_input_network->output->acti_vals[i_index]);
			}
		}
		this->action->activate(inner_flat_vals[this->nodes.size()],
							   action_input,
							   predicted_score);
		obs_input = this->action->outputs;
	}
	fold_input.push_back(vector<double>());	// empty
	for (int i_index = (int)this->nodes.size()+1; i_index < this->sequence_length; i_index++) {
		if (this->compound_actions[i_index] != NULL) {
			input_fold_inputs[i_index].push_back(vector<double>());	// empty
		}
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

	vector<double> compression_inputs;
	for (int l_index = (int)state_vals.size()-this->compress_num_layers; l_index < (int)state_vals.size(); l_index++) {
		for (int st_index = 0; st_index < (int)state_vals[l_index].size(); st_index++) {
			compression_inputs.push_back(state_vals[l_index][st_index]);
		}
	}
	for (int st_index = 0; st_index < (int)s_input_vals[state_vals.size()-this->compress_num_layers].size(); st_index++) {
		compression_inputs.push_back(s_input_vals[state_vals.size()-this->compress_num_layers][st_index]);
	}
	this->small_compression_network->activate(compression_inputs);

	for (int s_index = 0; s_index < this->compress_num_layers; s_index++) {
		state_vals.pop_back();
		s_input_vals.pop_back();
	}
	state_vals.push_back(vector<double>(this->compress_new_size));
	s_input_vals.push_back(vector<double>());

	for (int st_index = 0; st_index < (int)state_vals.back().size(); st_index++) {
		state_vals.back()[st_index] = this->small_compression_network->output->acti_vals[st_index];
	}

	for (int f_index = (int)this->nodes.size()+1; f_index < this->sequence_length; f_index++) {
		if (this->compound_actions[f_index] == NULL) {
			fold_input.push_back(flat_vals[f_index]);
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->compound_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(flat_vals[f_index]);
				}
			}
		} else {
			this->curr_scope_input_folds[f_index]->activate(input_fold_inputs[f_index]);
			vector<double> scope_input(this->compound_actions[f_index]->num_inputs);
			for (int i_index = 0; i_index < this->compound_actions[f_index]->num_inputs; i_index++) {
				scope_input[i_index] = this->curr_scope_input_folds[f_index]->output->acti_vals[i_index];
			}

			double scope_predicted_score = 0.0;
			this->compound_actions[f_index]->activate(inner_flat_vals[f_index],
													  scope_input,
													  scope_predicted_score);

			vector<double> scope_outputs = this->compound_actions[f_index]->outputs;
			scope_outputs.push_back(scope_predicted_score);

			fold_input.push_back(scope_outputs);
			for (int i_index = f_index+1; i_index < this->sequence_length; i_index++) {
				if (this->compound_actions[i_index] != NULL) {
					input_fold_inputs[i_index].push_back(scope_outputs);
				}
			}
		}
	}

	this->curr_fold->activate(fold_input,
							  state_vals);

	vector<double> errors;
	errors.push_back((target_val-predicted_score) - this->curr_fold->output->acti_vals[0]);
	this->sum_error += errors[0]*errors[0];

	this->curr_fold->backprop_last_state_with_no_weight_change(errors);

	vector<double> last_scope_state_errors(this->curr_scope_sizes.back());
	for (int st_index = 0; st_index < this->curr_scope_sizes.back(); st_index++) {
		last_scope_state_errors[st_index] = this->curr_fold->state_inputs.back()->errors[st_index];
		this->curr_fold->state_inputs.back()->errors[st_index] = 0.0;
	}

	vector<vector<double>> scope_input_errors(this->sequence_length);
	vector<double> scope_predicted_score_errors(this->sequence_length, 0.0);
	for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
		if (this->compound_actions[f_index] != NULL) {
			scope_input_errors[f_index] = vector<double>(this->compound_actions[f_index]->num_outputs, 0.0);
			for (int i_index = 0; i_index < this->compound_actions[f_index]->num_outputs; i_index++) {
				scope_input_errors[f_index][i_index] = this->curr_fold->flat_inputs[f_index]->errors[i_index];
				this->curr_fold->flat_inputs[f_index]->errors[i_index] = 0.0;
			}
			scope_predicted_score_errors[f_index] += this->curr_fold->flat_inputs[f_index]->errors[this->compound_actions[f_index]->num_outputs];
			this->curr_fold->flat_inputs[f_index]->errors[this->compound_actions[f_index]->num_outputs] = 0.0;
		}
	}
	for (int f_index = this->sequence_length-1; f_index >= (int)this->nodes.size()+1; f_index--) {
		if (this->compound_actions[f_index] != NULL) {
			vector<double> scope_output_errors;
			this->compound_actions[f_index]->backprop_errors_with_no_weight_change(
				scope_input_errors[f_index],
				scope_output_errors,
				scope_predicted_score_errors[f_index]);
			this->curr_scope_input_folds[f_index]->backprop_last_state_with_no_weight_change(scope_output_errors);

			for (int st_index = 0; st_index < this->test_scope_sizes.back(); st_index++) {
				last_scope_state_errors[st_index] += this->curr_scope_input_folds[f_index]->state_inputs.back()->errors[st_index];
				this->curr_scope_input_folds[f_index]->state_inputs.back()->errors[st_index] = 0.0;
			}

			for (int ff_index = f_index-1; ff_index >= (int)this->nodes.size()+1; ff_index--) {
				if (this->compound_actions[ff_index] != NULL) {
					for (int i_index = 0; i_index < this->compound_actions[ff_index]->num_outputs; i_index++) {
						scope_input_errors[ff_index][i_index] = this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index];
						this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[i_index] = 0.0;
					}
					scope_predicted_score_errors[ff_index] += this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[this->compound_actions[ff_index]->num_outputs];
					this->curr_scope_input_folds[f_index]->flat_inputs[ff_index]->errors[this->compound_actions[ff_index]->num_outputs] = 0.0;
				}
			}
		}
	}

	if (this->stage_iter <= 80000) {
		this->small_compression_network->backprop_weights_with_no_error_signal(
			last_scope_state_errors,
			0.05);
	} else if (this->stage_iter <= 160000) {
		this->small_compression_network->backprop_weights_with_no_error_signal(
			last_scope_state_errors,
			0.01);
	} else {
		this->small_compression_network->backprop_weights_with_no_error_signal(
			last_scope_state_errors,
			0.002);
	}
}
