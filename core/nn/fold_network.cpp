#include "fold_network.h"

using namespace std;

FoldNetwork::FoldNetwork(int existing_state_size,
						 int flat_size,
						 int new_state_size,
						 int obs_size) {
	this->existing_state_size = existing_state_size;
	this->flat_size = flat_size;
	this->new_state_size = new_state_size;
	this->obs_size = obs_size;

	for (int i_index = 0; i_index < this->flat_size; i_index++) {
		this->input_on.push_back(true);
	}

	this->existing_state_input = new Layer(LINEAR_LAYER, this->existing_state_size);
	this->flat_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->activated_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->new_state_input = new Layer(LINEAR_LAYER, this->new_state_size);
	this->obs_input = new Layer(LINEAR_LAYER, this->obs_size);

	int total_input_size = this->existing_state_size + 2*this->flat_size \
		+ this->new_state_size + this->obs_size;
	this->hidden = new Layer(RELU_LAYER, total_input_size*total_input_size);
	this->hidden->input_layers.push_back(this->existing_state_input);
	this->hidden->input_layers.push_back(this->flat_input);
	this->hidden->input_layers.push_back(this->activated_input);
	this->hidden->input_layers.push_back(this->new_state_input);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch = 0;
	this->iter = 0;
}

FoldNetwork::~FoldNetwork() {
	delete this->existing_state_input;
	delete this->flat_input;
	delete this->activated_input;
	delete this->new_state_input;
	delete this->obs_input;

	delete this->hidden;
	delete this->output;
}

void FoldNetwork::activate(vector<double>& existing_state_inputs,
						   double* flat_inputs,
						   bool* activated,
						   double* new_state_vals,
						   vector<double>& obs) {
	for (int e_index = 0; e_index < this->existing_state_size; e_index++) {
		this->existing_state_input->acti_vals[e_index] = existing_state_inputs[e_index];
	}

	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		this->flat_input->acti_vals[f_index] = flat_inputs[f_index];

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		this->new_state_input->acti_vals[s_index] = new_state_vals[s_index];
	}

	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = obs[o_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::full_backprop(vector<double>& errors,
								double* new_state_errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		new_state_errors[s_index] = this->new_state_input->errors[s_index];
		this->new_state_input->errors[s_index] = 0.0;
	}

	if (this->iter == 100) {
		double max_update = 0.0;
		full_calc_max_update(max_update,
							 0.001,
							 0.2);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		full_update_weights(factor,
							0.001,
							0.2);

		this->epoch++;
		this->iter = 0;
	} else {
		this->iter++;
	}
}

void FoldNetwork::full_calc_max_update(double& max_update,
									   double learning_rate,
									   double momentum) {
	this->hidden->calc_max_update(max_update,
								  learning_rate,
								  momentum);
	this->output->calc_max_update(max_update,
								  learning_rate,
								  momentum);
}

void FoldNetwork::full_update_weights(double factor,
									  double learning_rate,
									  double momentum) {
	this->hidden->update_weights(factor,
								 learning_rate,
								 momentum);
	this->output->update_weights(factor,
								 learning_rate,
								 momentum);
}

void FoldNetwork::state_backprop(std::vector<double>& errors,
								 double* new_state_errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop_fold_state();

	for (int s_index = 0; s_index < this->new_state_size; s_index++) {
		new_state_errors[s_index] = this->new_state_input->errors[s_index];
		this->new_state_input->errors[s_index] = 0.0;
	}

	if (this->iter == 100) {
		double max_update = 0.0;
		state_calc_max_update(max_update,
							  0.001,
							  0.2);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		state_update_weights(factor,
							 0.001,
							 0.2);

		this->epoch++;
		this->iter = 0;
	} else {
		this->iter++;
	}
}

void FoldNetwork::state_calc_max_update(double& max_update,
										double learning_rate,
										double momentum) {
	this->hidden->calc_max_update_fold_state(max_update,
											 learning_rate,
											 momentum);
	this->output->calc_max_update(max_update,
								  learning_rate,
								  momentum);
}

void FoldNetwork::state_update_weights(double factor,
									   double learning_rate,
									   double momentum) {
	this->hidden->update_weights_fold_state(factor,
											learning_rate,
											momentum);
	this->output->update_weights(factor,
								 learning_rate,
								 momentum);
}
