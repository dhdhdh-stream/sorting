#include "fold_network.h"

using namespace std;

FoldNetwork::FoldNetwork(vector<Node*> input_mappings,
						 int state_size) {
	this->input_mappings = input_mappings;
	this->state_size = state_size;

	this->flat_input = new Layer(LINEAR_LAYER, (int)this->input_mappings.size());
	this->activated_input = new Layer(LINEAR_LAYER, (int)this->input_mappings.size());
	this->state_input = new Layer(LINEAR_LAYER, this->state_size);

	this->hidden = new Layer(RELU_LAYER, 1000);
	this->hidden->input_layers.push_back(this->flat_input);
	this->hidden->input_layers.push_back(this->activated_input);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch = 0;
	this->iter = 0;
}

FoldNetwork::FoldNetwork(std::vector<Node*> input_mappings,
						 int state_size,
						 ifstream& input_file) {
	this->input_mappings = input_mappings;
	this->state_size = state_size;

	this->flat_input = new Layer(LINEAR_LAYER, (int)this->input_mappings.size());
	this->activated_input = new Layer(LINEAR_LAYER, (int)this->input_mappings.size());
	this->state_input = new Layer(LINEAR_LAYER, this->state_size);

	this->hidden = new Layer(RELU_LAYER, 1000);
	this->hidden->input_layers.push_back(this->flat_input);
	this->hidden->input_layers.push_back(this->activated_input);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch = 0;
	this->iter = 0;

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

FoldNetwork::~FoldNetwork() {
	delete this->flat_input;
	delete this->activated_input;
	delete this->state_input;

	delete this->hidden;
	delete this->output;
}

void FoldNetwork::activate(vector<double>& inputs,
						   vector<bool>& activated,
						   double* state_vals) {
	for (int i_index = 0; i_index < (int)this->input_mappings.size(); i_index++) {
		if (this->input_mappings[i_index]->network_on) {
			this->flat_input->acti_vals[i_index] = 0.0;
		} else {
			this->flat_input->acti_vals[i_index] = inputs[i_index];
		}

		if (activated[i_index]) {
			this->activated_input->acti_vals[i_index] = 1.0;
		} else {
			this->activated_input->acti_vals[i_index] = 0.0;
		}
	}

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::full_backprop(vector<double>& errors,
								double* state_errors) {
	this->output->errors[0] = errors[0];

	this->output->backprop();
	this->hidden->backprop();

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_errors[s_index] = this->state_input->errors[s_index];
		this->state_input->errors[s_index] = 0.0;
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
								 double* state_errors) {
	this->output->errors[0] = errors[0];

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_fold_state();

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_errors[s_index] = this->state_input->errors[s_index];
		this->state_input->errors[s_index] = 0.0;
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
	this->output->calc_max_update_fold_state(max_update,
											 learning_rate,
											 momentum);
}

void FoldNetwork::state_update_weights(double factor,
									   double learning_rate,
									   double momentum) {
	this->hidden->update_weights_fold_state(factor,
											learning_rate,
											momentum);
	this->output->update_weights_fold_state(factor,
											learning_rate,
											momentum);
}

void FoldNetwork::save(ofstream& output_file) {
	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}
