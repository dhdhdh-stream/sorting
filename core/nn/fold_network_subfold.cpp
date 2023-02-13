#include "fold_network.h"

#include <iostream>

using namespace std;

void FoldNetwork::set_s_input_size(int s_input_size) {
	this->s_input_size = s_input_size;
	delete this->s_input_input;
	this->s_input_input = new Layer(LINEAR_LAYER, s_input_size);
	this->hidden->subfold_set_s_input((int)this->flat_sizes.size(),
									  this->s_input_input);
}

void FoldNetwork::activate_subfold(vector<double>& s_input_vals,
								   vector<vector<double>>& state_vals) {
	for (int st_index = 0; st_index < this->s_input_size; st_index++) {
		this->s_input_input->acti_vals[st_index] = s_input_vals[st_index];
	}

	for (int sc_index = this->subfold_index+1; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	this->hidden->subfold_activate(this->subfold_index,
								   (int)this->state_inputs.size());
	this->output->activate();
}

void FoldNetwork::activate_subfold(vector<double>& s_input_vals,
								   vector<vector<double>>& state_vals,
								   FoldNetworkHistory* history) {
	activate_subfold(s_input_vals,
					 state_vals);

	history->save_weights();
}

void FoldNetwork::backprop_subfold_errors_with_no_weight_change(vector<double>& errors) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->subfold_backprop_errors_with_no_weight_change(
		this->subfold_index,
		(int)this->state_inputs.size());
}

void FoldNetwork::backprop_subfold_errors_with_no_weight_change(vector<double>& errors,
																FoldNetworkHistory* history) {
	history->reset_weights();

	backprop_subfold_errors_with_no_weight_change(errors);
}

void FoldNetwork::backprop_subfold_weights_with_no_error_signal(
		vector<double>& errors,
		double target_max_update) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->subfold_backprop_weights_with_no_error_signal(
		this->subfold_index,
		(int)this->state_inputs.size());

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->subfold_get_max_update(this->subfold_index,
											 (int)this->state_inputs.size(),
											 hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->subfold_update_weights(this->subfold_index,
												 (int)this->state_inputs.size(),
												 hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > target_max_update) {
				output_learning_rate = target_max_update/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void FoldNetwork::backprop_subfold_weights_with_no_error_signal(
		vector<double>& errors,
		double target_max_update,
		FoldNetworkHistory* history) {
	history->reset_weights();

	backprop_subfold_weights_with_no_error_signal(errors,
												  target_max_update);
}

void FoldNetwork::backprop_subfold_new_s_input(vector<double>& errors,
											   double target_max_update) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->subfold_backprop_s_input(this->subfold_index,
										  (int)this->state_inputs.size());

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->subfold_get_max_update(this->subfold_index,
											 (int)this->state_inputs.size(),
											 hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->subfold_update_weights(this->subfold_index,
												 (int)this->state_inputs.size(),
												 hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > target_max_update) {
				output_learning_rate = target_max_update/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void FoldNetwork::backprop_subfold_new_s_input(vector<double>& errors,
											   double target_max_update,
											   FoldNetworkHistory* history) {
	history->reset_weights();

	backprop_subfold_new_s_input(errors,
								 target_max_update);
}
