#include "collapse_network.h"

#include <iostream>

using namespace std;

CollapseNetwork::CollapseNetwork(vector<int> time_sizes,
								 int global_size) {
	for (int i_index = 0; i_index < (int)time_sizes.size(); i_index++) {
		this->time_inputs.push_back(new Layer(LINEAR_LAYER, time_sizes[i_index]));
	}
	this->global_input = new Layer(LINEAR_LAYER, global_size);

	this->state = FLAT;
	this->index = -1;

	this->process = new Layer(RELU_LAYER, 100);
	for (int i_index = 0; i_index < (int)this->time_inputs.size(); i_index++) {
		this->process->input_layers.push_back(this->time_inputs[i_index]);
	}
	this->process->input_layers.push_back(global_input);
	this->process->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->process);
	this->output->setup_weights_full();
}

CollapseNetwork::CollapseNetwork(CollapseNetwork* original) {
	for (int i_index = 0; i_index < (int)original->time_inputs.size(); i_index++) {
		int input_size = (int)original->time_inputs[i_index]->acti_vals.size();
		this->time_inputs.push_back(new Layer(LINEAR_LAYER, input_size));
	}
	int global_input_size = (int)original->global_input->acti_vals.size();
	this->global_input = new Layer(LINEAR_LAYER, global_input_size);

	this->process = new Layer(RELU_LAYER, 100);
	if (original->state == FLAT) {
		for (int i_index = 0; i_index < (int)this->time_inputs.size(); i_index++) {
			this->process->input_layers.push_back(this->time_inputs[i_index]);
		}
	}
	else if (original->state == COLLAPSE) {
		for (int c_index = 0; c_index < original->index+1; c_index++) {
			this->collapse_layers.push_back(new Layer(RELU_LAYER, 20));
			this->collapse_layers[c_index]->input_layers.push_back(this->time_inputs[c_index]);
			this->collapse_layers[c_index]->input_layers.push_back(this->global_input);
			this->collapse_layers[c_index]->setup_weights_full();
			this->collapse_layers[c_index]->copy_weights_from(original->collapse_layers[c_index]);

			int output_node_size = (int)original->collapse_outputs[c_index]->acti_vals.size();
			this->collapse_outputs.push_back(new Layer(LINEAR_LAYER, output_node_size));
			this->collapse_outputs[c_index]->input_layers.push_back(this->collapse_layers[c_index]);
			this->collapse_outputs[c_index]->setup_weights_full();
			this->collapse_outputs[c_index]->copy_weights_from(original->collapse_outputs[c_index]);

			this->process->input_layers.push_back(this->collapse_outputs[c_index]);
		}
		for (int i_index = original->index+1; i_index < (int)this->time_inputs.size(); i_index++) {
			this->process->input_layers.push_back(this->time_inputs[i_index]);
		}
	} else {
		for (int c_index = 0; c_index < (int)this->time_inputs.size(); c_index++) {
			this->collapse_layers.push_back(new Layer(RELU_LAYER, 20));
			this->collapse_layers[c_index]->input_layers.push_back(this->time_inputs[c_index]);
			this->collapse_layers[c_index]->input_layers.push_back(this->global_input);
			this->collapse_layers[c_index]->setup_weights_full();
			this->collapse_layers[c_index]->copy_weights_from(original->collapse_layers[c_index]);

			int output_node_size = (int)original->collapse_outputs[c_index]->acti_vals.size();
			this->collapse_outputs.push_back(new Layer(LINEAR_LAYER, output_node_size));
			this->collapse_outputs[c_index]->input_layers.push_back(this->collapse_layers[c_index]);
			this->collapse_outputs[c_index]->setup_weights_full();
			this->collapse_outputs[c_index]->copy_weights_from(original->collapse_outputs[c_index]);
		}

		for (int f_index = 0; f_index < original->index+1; f_index++) {
			this->fold_layers.push_back(new Layer(RELU_LAYER, 20));
			this->fold_layers[f_index]->input_layers.push_back(this->collapse_outputs[f_index]);
			this->fold_layers[f_index]->input_layers.push_back(this->collapse_outputs[f_index+1]);
			this->fold_layers[f_index]->input_layers.push_back(this->global_input);
			this->fold_layers[f_index]->setup_weights_full();
			this->fold_layers[f_index]->copy_weights_from(original->fold_layers[f_index]);

			int output_node_size = (int)original->fold_outputs[f_index]->acti_vals.size();
			this->fold_outputs.push_back(new Layer(LINEAR_LAYER, output_node_size));
			this->fold_outputs[f_index]->input_layers.push_back(this->fold_layers[f_index]);
			this->fold_outputs[f_index]->setup_weights_full();
			this->fold_outputs[f_index]->copy_weights_from(original->fold_outputs[f_index]);

			this->process->input_layers.push_back(this->fold_outputs[f_index]);
		}

		for (int c_index = original->index+2; c_index < (int)this->time_inputs.size(); c_index++) {
			this->process->input_layers.push_back(this->collapse_outputs[c_index]);
		}
	}
	this->process->input_layers.push_back(this->global_input);
	this->process->setup_weights_full();
	this->process->copy_weights_from(original->process);

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->process);
	this->output->setup_weights_full();
	this->output->copy_weights_from(original->output);

	this->state = original->state;
	this->index = original->index;
}

CollapseNetwork::~CollapseNetwork() {
	for (int i = 0; i < (int)this->time_inputs.size(); i++) {
		delete this->time_inputs[i];
	}
	delete this->global_input;

	for (int i = 0; i < (int)this->collapse_layers.size(); i++) {
		delete this->collapse_layers[i];
	}
	for (int i = 0; i < (int)this->collapse_outputs.size(); i++) {
		delete this->collapse_outputs[i];
	}

	for (int i = 0; i < (int)this->fold_layers.size(); i++) {
		delete this->fold_layers[i];
	}
	for (int i = 0; i < (int)this->fold_outputs.size(); i++) {
		delete this->fold_outputs[i];
	}

	delete this->process;
	delete this->output;
}

void CollapseNetwork::activate(vector<vector<double>>& time_vals,
							   vector<double>& global_vals) {
	for (int t_index = 0; t_index < (int)time_vals.size(); t_index++) {
		for (int i_index = 0; i_index < (int)time_vals[t_index].size(); i_index++) {
			this->time_inputs[t_index]->acti_vals[i_index] = time_vals[t_index][i_index];
		}
	}
	for (int i_index = 0; i_index < (int)global_vals.size(); i_index++) {
		this->global_input->acti_vals[i_index] = global_vals[i_index];
	}

	if (this->state == FLAT) {
		// do nothing
	} else if (this->state == COLLAPSE) {
		for (int c_index = 0; c_index < this->index+1; c_index++) {
			this->collapse_layers[c_index]->activate();
			this->collapse_outputs[c_index]->activate();
		}
	} else {
		for (int c_index = 0; c_index < (int)this->time_inputs.size(); c_index++) {
			this->collapse_layers[c_index]->activate();
			this->collapse_outputs[c_index]->activate();
		}
		for (int f_index = 0; f_index < this->index+1; f_index++) {
			this->fold_layers[f_index]->activate();
			this->fold_outputs[f_index]->activate();
		}
	}

	this->process->activate();
	this->output->activate();
}

void CollapseNetwork::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->process->backprop();

	if (this->state == FLAT) {
		// do nothing
	} else if (this->state == COLLAPSE) {
		this->collapse_outputs[this->index]->backprop();
		this->collapse_layers[this->index]->backprop();
	} else {
		this->fold_outputs[this->index]->backprop();
		this->fold_layers[this->index]->backprop();
	}
}

void CollapseNetwork::calc_max_update(double& max_update,
									  double learning_rate,
									  double momentum) {
	if (this->state == FLAT) {
		this->process->calc_max_update(-1,
									   max_update,
									   learning_rate,
									   momentum);
		this->output->calc_max_update(-1,
									  max_update,
									  learning_rate,
									  momentum);
	} else if (this->state == COLLAPSE) {
		this->process->calc_max_update(this->index,
									   max_update,
									   learning_rate,
									   momentum);

		this->collapse_layers[this->index]->calc_max_update(
			-1,
			max_update,
			learning_rate,
			momentum);
		this->collapse_outputs[this->index]->calc_max_update(
			-1,
			max_update,
			learning_rate,
			momentum);
	} else {
		this->process->calc_max_update(this->index,
									   max_update,
									   learning_rate,
									   momentum);

		this->fold_layers[this->index]->calc_max_update(
			-1,
			max_update,
			learning_rate,
			momentum);
		this->fold_outputs[this->index]->calc_max_update(
			-1,
			max_update,
			learning_rate,
			momentum);
	}
}

void CollapseNetwork::update_weights(double factor,
									 double learning_rate,
									 double momentum) {
	if (this->state == FLAT) {
		this->process->update_weights(-1,
									  factor,
									  learning_rate,
									  momentum);
		this->output->update_weights(-1,
									 factor,
									 learning_rate,
									 momentum);
	} else if (this->state == COLLAPSE) {
		this->process->update_weights(this->index,
									  factor,
									  learning_rate,
									  momentum);

		this->collapse_layers[this->index]->update_weights(
			-1,
			factor,
			learning_rate,
			momentum);
		this->collapse_outputs[this->index]->update_weights(
			-1,
			factor,
			learning_rate,
			momentum);
	} else {
		this->process->update_weights(this->index,
									  factor,
									  learning_rate,
									  momentum);

		this->fold_layers[this->index]->update_weights(
			-1,
			factor,
			learning_rate,
			momentum);
		this->fold_outputs[this->index]->update_weights(
			-1,
			factor,
			learning_rate,
			momentum);
	}
}

void CollapseNetwork::next_step(int state_size) {
	if (this->state == FLAT) {
		cout << "FLAT to COLLAPSE" << endl;

		this->state = COLLAPSE;
		this->index = 0;

		this->collapse_layers.push_back(new Layer(RELU_LAYER, 20));
		this->collapse_layers[0]->input_layers.push_back(this->time_inputs[0]);
		this->collapse_layers[0]->input_layers.push_back(this->global_input);
		this->collapse_layers[0]->setup_weights_full();

		this->collapse_outputs.push_back(new Layer(LINEAR_LAYER, state_size));
		this->collapse_outputs[0]->input_layers.push_back(this->collapse_layers[0]);
		this->collapse_outputs[0]->setup_weights_full();

		this->process->collapse_input(0, this->collapse_outputs[0]);
	} else if (this->state == COLLAPSE) {
		if (this->index == (int)this->time_inputs.size()-1) {
			cout << "COLLAPSE to FOLD" << endl;

			this->state = FOLD;
			this->index = 0;

			this->fold_layers.push_back(new Layer(RELU_LAYER, 20));
			this->fold_layers[0]->input_layers.push_back(this->collapse_outputs[0]);
			this->fold_layers[0]->input_layers.push_back(this->collapse_outputs[1]);
			this->fold_layers[0]->input_layers.push_back(this->global_input);
			this->fold_layers[0]->setup_weights_full();

			this->fold_outputs.push_back(new Layer(LINEAR_LAYER, state_size));
			this->fold_outputs[0]->input_layers.push_back(this->fold_layers[0]);
			this->fold_outputs[0]->setup_weights_full();

			this->process->fold_input(this->fold_outputs[0]);
		} else {
			cout << "FOLD #" << this->index << " to FOLD #" << this->index+1 << endl;

			this->index++;

			this->collapse_layers.push_back(new Layer(RELU_LAYER, 20));
			this->collapse_layers[this->index]->input_layers.push_back(this->time_inputs[this->index]);
			this->collapse_layers[this->index]->input_layers.push_back(this->global_input);
			this->collapse_layers[this->index]->setup_weights_full();

			this->collapse_outputs.push_back(new Layer(LINEAR_LAYER, state_size));
			this->collapse_outputs[this->index]->input_layers.push_back(this->collapse_layers[this->index]);
			this->collapse_outputs[this->index]->setup_weights_full();

			this->process->collapse_input(this->index, this->collapse_outputs[this->index]);
		}
	}
}
