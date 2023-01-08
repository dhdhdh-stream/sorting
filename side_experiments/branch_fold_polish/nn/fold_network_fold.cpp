#include "fold_network.h"

#include <iostream>

using namespace std;

void FoldNetwork::add_scope(int scope_size) {
	this->scope_sizes.push_back(scope_size);
	this->state_inputs.push_back(new Layer(LINEAR_LAYER, scope_size));
	this->hidden->fold_add_scope(this->state_inputs.back());
}

void FoldNetwork::pop_scope() {
	this->scope_sizes.pop_back();
	delete this->state_inputs.back();
	this->state_inputs.pop_back();
	this->hidden->fold_pop_scope();
}

void FoldNetwork::migrate_weights() {
	for (int n_index = 0; n_index < (int)this->hidden->acti_vals.size(); n_index++) {
		for (int s_index = 0; s_index < this->scope_sizes.back(); s_index++)
			this->hidden->weights[n_index].back()[s_index] = this->hidden->weights[n_index][this->fold_index][s_index];
	}
}

void FoldNetwork::activate_fold(vector<vector<double>>& flat_vals,
								vector<double>& outer_s_input_vals,
								vector<vector<double>>& state_vals) {
	for (int f_index = this->fold_index+1; f_index < (int)this->flat_sizes.size(); f_index++) {
		for (int s_index = 0; s_index < this->flat_sizes[f_index]; s_index++) {
			this->flat_inputs[f_index]->acti_vals[s_index] = flat_vals[f_index][s_index];
		}
	}
	for (int s_index = 0; s_index < this->s_input_size; s_index++) {
		this->s_input_input->acti_vals[s_index] = outer_s_input_vals[s_index];
	}
	for (int sc_index = 0; sc_index < (int)state_vals.size(); sc_index++) {
		for (int st_index = 0; st_index < (int)state_vals[sc_index].size(); st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}

	this->hidden->fold_activate(this->fold_index);
	this->output->activate();
}

void FoldNetwork::activate_fold(vector<vector<double>>& flat_vals,
								vector<double>& outer_s_input_vals,
								vector<vector<double>>& state_vals,
								FoldNetworkHistory* history) {
	activate_fold(flat_vals,
				  outer_s_input_vals,
				  state_vals);

	history->save_weights();
}

void FoldNetwork::backprop_fold_errors_with_no_weight_change(vector<double>& errors) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();	// don't special case for now
}

void FoldNetwork::backprop_fold_errors_with_no_weight_change(vector<double>& errors,
															 FoldNetworkHistory* history) {
	history->reset_weights();

	backprop_fold_errors_with_no_weight_change(errors);
}

void FoldNetwork::backprop_fold_weights_with_no_error_signal(vector<double>& errors,
															 double target_max_update) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->fold_backprop_weights_with_no_error_signal(this->fold_index);

	this->epoch_iter++;
	// if (this->epoch_iter == 20) {
	if (this->epoch_iter == 5) {
		double hidden_max_update = 0.0;
		this->hidden->fold_get_max_update(this->fold_index,
										  hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->fold_update_weights(this->fold_index,
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

void FoldNetwork::backprop_fold_weights_with_no_error_signal(vector<double>& errors,
															 double target_max_update,
															 FoldNetworkHistory* history) {
	history->reset_weights();

	backprop_fold_weights_with_no_error_signal(errors,
											   target_max_update);
}

void FoldNetwork::backprop_fold_last_state(vector<double>& errors,
										   double target_max_update) {
	for (int e_index = 0; e_index < this->output_size; e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->fold_backprop_last_state(this->fold_index);

	this->epoch_iter++;
	// if (this->epoch_iter == 20) {
	if (this->epoch_iter == 5) {
		double hidden_max_update = 0.0;
		this->hidden->fold_get_max_update(this->fold_index,
										  hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->fold_update_weights(this->fold_index,
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

void FoldNetwork::backprop_fold_last_state(vector<double>& errors,
										   double target_max_update,
										   FoldNetworkHistory* history) {
	history->reset_weights();

	backprop_fold_last_state(errors,
							 target_max_update);
}
