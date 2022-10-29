#include "sub_fold_network.h"

#include <iostream>

using namespace std;

void SubFoldNetwork::construct() {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->state_inputs.push_back(new Layer(LINEAR_LAYER, this->scope_sizes[sc_index]));
	}
	for (int sc_index = 0; sc_index < (int)this->s_input_sizes.size(); sc_index++) {
		this->s_input_inputs.push_back(new Layer(LINEAR_LAYER, this->s_input_sizes[sc_index]));
	}

	this->hidden = new Layer(LEAKY_LAYER, 100);	// set fixed number because scope_sizes will change
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->state_inputs[sc_index]);
	}
	for (int sc_index = 0; sc_index < (int)this->s_input_sizes.size(); sc_index++) {
		this->hidden->input_layers.push_back(this->s_input_inputs[sc_index]);
	}
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, this->output_size);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

SubFoldNetwork::SubFoldNetwork(vector<int> scope_sizes,
							   vector<int> s_input_sizes,
							   int output_size) {
	this->scope_sizes = scope_sizes;
	this->s_input_sizes = s_input_sizes;
	this->output_size = output_size;

	this->fold_index = -1;

	construct();
}

SubFoldNetwork::SubFoldNetwork(SubFoldNetwork* original) {
	this->scope_sizes = original->scope_sizes;
	this->s_input_sizes = original->s_input_sizes;
	this->output_size = original->output_size;

	this->fold_index = original->fold_index;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

SubFoldNetwork::~SubFoldNetwork() {
	for (int sc_index = 0; sc_index < (int)this->state_inputs.size(); sc_index++) {
		delete this->state_inputs[sc_index];
	}
	for (int sc_index = 0; sc_index < (int)this->s_input_inputs.size(); sc_index++) {
		delete this->s_input_inputs[sc_index];
	}

	delete this->hidden;
	delete this->output;
}

void SubFoldNetwork::add_s_input(int layer,
								 int num_state) {
	this->s_input_sizes[layer] += num_state;
	for (int s_index = 0; s_index < num_state; s_index++) {
		this->s_input_inputs[layer]->acti_vals.push_back(0.0);
		this->s_input_inputs[layer]->errors.push_back(0.0);
	}

	this->hidden->subfold_add_s_input((int)this->state_inputs.size()+layer,
									  num_state);
}

void SubFoldNetwork::activate(vector<vector<double>>& state_vals,
							  vector<vector<double>>& s_input_vals) {
	for (int sc_index = 0; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}
	for (int sc_index = 0; sc_index < (int)this->s_input_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->s_input_sizes[sc_index]; st_index++) {
			this->s_input_inputs[sc_index]->acti_vals[st_index] = s_input_vals[sc_index][st_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void SubFoldNetwork::activate(vector<vector<double>>& state_vals,
							  vector<vector<double>>& s_input_vals) {
	for (int sc_index = this->fold_index+1; sc_index < (int)this->scope_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->scope_sizes[sc_index]; st_index++) {
			this->state_inputs[sc_index]->acti_vals[st_index] = state_vals[sc_index][st_index];
		}
	}
	for (int sc_index = this->fold_index+1; sc_index < (int)this->s_input_sizes.size(); sc_index++) {
		for (int st_index = 0; st_index < this->s_input_sizes[sc_index]; st_index++) {
			this->s_input_inputs[sc_index]->acti_vals[st_index] = s_input_vals[sc_index][st_index];
		}
	}

	this->hidden->subfold_activate(this->fold_index,
								   (int)this->state_inputs.size());
	this->output->activate();
}

void SubFoldNetwork::backprop_weights_with_no_error_signal(
		vector<double>& errors,
		double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
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

void SubFoldNetwork::backprop_new_s_input(int layer,
										  int new_input_size,
										  vector<double>& errors,
										  double target_max_update) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->subfold_backprop_new_s_input(this->fold_index,
											   (int)this->state_inputs.size(),
											   (int)this->state_inputs.size()+layer,
											   new_input_size);

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->subfold_get_max_update(this->fold_index,
											 (int)this->state_inputs.size(),
											 hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > target_max_update) {
				hidden_learning_rate = target_max_update/hidden_max_update;
			}
			this->hidden->subfold_update_weights(this->fold_index,
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

// void SubFoldNetwork::backprop(vector<double>& errors,
// 							  double target_max_update) {
// 	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
// 		this->output->errors[e_index] = errors[e_index];
// 	}

// 	this->output->backprop();
// 	this->hidden->subfold_backprop(this->fold_index);

// 	this->epoch_iter++;
// 	if (this->epoch_iter == 20) {
// 		double hidden_max_update = 0.0;
// 		this->hidden->subfold_get_max_update(this->fold_index,
// 											 hidden_max_update);
// 		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
// 		if (hidden_max_update > 0.0) {
// 			double hidden_learning_rate = (0.3*target_max_update)/this->hidden_average_max_update;
// 			if (hidden_learning_rate*hidden_max_update > target_max_update) {
// 				hidden_learning_rate = target_max_update/hidden_max_update;
// 			}
// 			this->hidden->subfold_update_weights(this->fold_index,
// 												 hidden_learning_rate);
// 		}

// 		double output_max_update = 0.0;
// 		this->output->get_max_update(output_max_update);
// 		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
// 		if (output_max_update > 0.0) {
// 			double output_learning_rate = (0.3*target_max_update)/this->output_average_max_update;
// 			if (output_learning_rate*output_max_update > target_max_update) {
// 				output_learning_rate = target_max_update/output_max_update;
// 			}
// 			this->output->update_weights(output_learning_rate);
// 		}

// 		this->epoch_iter = 0;
// 	}
// }
