#include "fold_network.h"

#include <iostream>

using namespace std;

void FoldNetwork::activate_small(std::vector<double>& s_input_vals,
								 std::vector<double>& state_vals) {
	for (int st_index = 0; st_index < this->s_input_size; st_index++) {
		this->s_input_input->acti_vals[st_index] = s_input_vals[st_index];
	}

	int state_vals_index = 0;
	for (int sc_index = this->subfold_index+1; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[state_vals_index];
		}
	}

	this->hidden->subfold_activate(this->subfold_index,
								   (int)this->state_inputs.size());
	this->output->activate();
}

void FoldNetwork::backprop_small(vector<double>& errors,
								 double target_max_update,
								 vector<double>& s_input_errors,
								 vector<double>& state_errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->subfold_backprop(this->subfold_index,
								   (int)this->state_inputs.size());

	for (int st_index = 0; st_index < this->s_input_size; st_index++) {
		s_input_errors.push_back(this->s_input_input->errors[st_index]);
		this->s_input_input->errors[st_index] = 0.0;
	}

	for (int sc_index = this->subfold_index+1; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
			state_errors.push_back(this->state_inputs[sc_index]->errors[st_index]);
			this->state_inputs[sc_index]->errors[st_index] = 0.0;
		}
	}

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
		double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
		if (output_learning_rate*output_max_update > target_max_update) {
			output_learning_rate = target_max_update/output_max_update;
		}
		this->output->update_weights(output_learning_rate);

		this->epoch_iter = 0;
	}
}
