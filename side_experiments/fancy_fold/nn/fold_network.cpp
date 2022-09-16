#include "fold_network.h"

#include <iostream>

using namespace std;

void FoldNetwork::construct() {
	this->flat_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->activated_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->obs_input = new Layer(LINEAR_LAYER, 1);

	this->state_input = new Layer(LINEAR_LAYER, this->state_size);

	this->next_state_size = 0;
	this->next_state_input = new Layer(LINEAR_LAYER, 0);

	int total_input_size = 2*this->flat_size + 1;
	this->hidden = new Layer(RELU_LAYER, 2*total_input_size*total_input_size);
	this->hidden->input_layers.push_back(this->flat_input);
	this->hidden->input_layers.push_back(this->activated_input);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->input_layers.push_back(this->next_state_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, this->output_size);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
}

FoldNetwork::FoldNetwork(int flat_size,
						 int output_size) {
	this->flat_size = flat_size;
	this->output_size = output_size;

	this->state_size = 0;

	construct();
}

FoldNetwork::FoldNetwork(int flat_size,
						 int output_size,
						 ifstream& input_file) {
	this->flat_size = flat_size;
	this->output_size = output_size;

	string state_size_line;
	getline(input_file, state_size_line);
	this->state_size = stoi(state_size_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

FoldNetwork::FoldNetwork(FoldNetwork* original) {
	this->flat_size = original->flat_size;
	this->output_size = original->output_size;

	this->state_size = original->state_size;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

FoldNetwork::~FoldNetwork() {
	delete this->flat_input;
	delete this->activated_input;
	delete this->obs_input;
	delete this->state_input;
	delete this->next_state_input;

	delete this->hidden;
	delete this->output;
}

void FoldNetwork::activate(double* flat_inputs,
						   bool* activated,
						   vector<double>& obs) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		this->flat_input->acti_vals[f_index] = flat_inputs[f_index];

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = 0.0;
	}
	for (int s_index = 0; s_index < this->next_state_size; s_index++) {
		this->next_state_input->acti_vals[s_index] = 0.0;
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::activate(double* flat_inputs,
						   bool* activated,
						   vector<double>& obs,
						   vector<AbstractNetworkHistory*>& network_historys) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		this->flat_input->acti_vals[f_index] = flat_inputs[f_index];

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = 0.0;
	}
	for (int s_index = 0; s_index < this->next_state_size; s_index++) {
		this->next_state_input->acti_vals[s_index] = 0.0;
	}

	this->hidden->activate();
	this->output->activate();

	FoldNetworkHistory* network_history = new FoldNetworkHistory(this);
	network_historys.push_back(network_history);
}

void FoldNetwork::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();

	if (this->epoch_iter == 100) {
		double max_update = 0.0;
		calc_max_update(max_update,
						0.001);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		update_weights(factor,
					   0.001);

		this->epoch_iter = 0;
	} else {
		this->epoch_iter++;
	}
}

void FoldNetwork::calc_max_update(double& max_update,
								  double learning_rate) {
	this->hidden->calc_max_update(max_update,
								  learning_rate);
	this->output->calc_max_update(max_update,
								  learning_rate);
}

void FoldNetwork::update_weights(double factor,
								 double learning_rate) {
	this->hidden->update_weights(factor,
								 learning_rate);
	this->output->update_weights(factor,
								 learning_rate);
}

void FoldNetwork::set_next_state_size(int next_state_size) {
	delete this->next_state_input;

	this->next_state_size = next_state_size;
	this->next_state_input = new Layer(LINEAR_LAYER, next_state_size);
	this->hidden->input_layers[4] = this->next_state_input;
	this->hidden->fold_setup_next();
}

void FoldNetwork::activate_compare_hidden_and_backprop(
		double* flat_inputs,
		bool* activated,
		int fold_index,
		std::vector<double>& obs,
		std::vector<double>& state_vals,
		std::vector<double>& next_state_vals) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		if (fold_index-1 >= f_index) {	// decrement by 1 for current
			this->flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->flat_input->acti_vals[f_index] = flat_inputs[f_index];
		}

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}
	for (int s_index = 0; s_index < this->next_state_size; s_index++) {
		this->next_state_input->acti_vals[s_index] = 0.0;
	}

	this->hidden->activate_as_linear();

	double hidden_curr_activations[this->hidden->acti_vals.size()];
	for (int n_index = 0; n_index < (int)this->hidden->acti_vals.size(); n_index++) {
		hidden_curr_activations[n_index] = this->hidden->acti_vals[n_index];
	}

	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		if (fold_index >= f_index) {
			this->flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->flat_input->acti_vals[f_index] = flat_inputs[f_index];
		}

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = 0.0;
	}
	for (int s_index = 0; s_index < this->next_state_size; s_index++) {
		this->next_state_input->acti_vals[s_index] = next_state_vals[s_index];
	}

	this->hidden->activate_as_linear();

	for (int n_index = 0; n_index < (int)this->hidden->acti_vals.size(); n_index++) {
		double error = hidden_curr_activations[n_index] - this->hidden->acti_vals[n_index];
		this->hidden->errors[n_index] = error;
	}

	this->hidden->fold_backprop_next();

	if (this->epoch_iter == 100) {
		double max_update = 0.0;
		next_calc_max_update(max_update, 0.001);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		next_update_weights(factor, 0.001);

		this->epoch_iter = 0;
	} else {
		this->epoch_iter++;
	}
}

void FoldNetwork::next_calc_max_update(double& max_update,
									   double learning_rate) {
	this->hidden->fold_calc_max_update_next(max_update,
											learning_rate);
}

void FoldNetwork::next_update_weights(double factor,
									  double learning_rate) {
	this->hidden->fold_update_weights_next(factor,
										   learning_rate);
}

void FoldNetwork::activate_full(double* flat_inputs,
								bool* activated,
								int fold_index,
								vector<double>& obs,
								vector<double>& next_state_vals) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		if (fold_index >= f_index) {
			this->flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->flat_input->acti_vals[f_index] = flat_inputs[f_index];
		}

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = 0.0;
	}
	for (int s_index = 0; s_index < this->next_state_size; s_index++) {
		this->next_state_input->acti_vals[s_index] = next_state_vals[s_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::activate_full(double* flat_inputs,
								bool* activated,
								int fold_index,
								vector<double>& obs,
								vector<double>& next_state_vals,
								vector<AbstractNetworkHistory*>& network_historys) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		if (fold_index >= f_index) {
			this->flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->flat_input->acti_vals[f_index] = flat_inputs[f_index];
		}

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = 0.0;
	}
	for (int s_index = 0; s_index < this->next_state_size; s_index++) {
		this->next_state_input->acti_vals[s_index] = next_state_vals[s_index];
	}

	this->hidden->activate();
	this->output->activate();

	FoldNetworkHistory* network_history = new FoldNetworkHistory(this);
	network_historys.push_back(network_history);
}

void FoldNetwork::take_next() {
	delete this->state_input;
	this->state_input = this->next_state_input;
	this->next_state_input = new Layer(LINEAR_LAYER, 0);

	this->state_size = this->next_state_size;
	this->next_state_size = 0;

	this->hidden->input_layers[3] = this->state_input;
	this->hidden->input_layers[4] = this->next_state_input;

	this->hidden->fold_take_next();
}

void FoldNetwork::discard_next() {
	delete this->next_state_input;
	this->next_state_input = new Layer(LINEAR_LAYER, 0);

	this->hidden->input_layers[4] = this->next_state_input;

	this->hidden->fold_discard_next();
}

void FoldNetwork::activate_curr(double* flat_inputs,
								bool* activated,
								int fold_index,
								std::vector<double>& obs,
								std::vector<double>& state_vals) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		if (fold_index-1 >= f_index) {	// decrement by 1 for current
			this->flat_input->acti_vals[f_index] = 0.0;
		} else {
			this->flat_input->acti_vals[f_index] = flat_inputs[f_index];
		}

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	this->obs_input->acti_vals[0] = obs[0];

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}
	for (int s_index = 0; s_index < this->next_state_size; s_index++) {
		this->next_state_input->acti_vals[s_index] = 0.0;
	}

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::save(ofstream& output_file) {
	output_file << this->state_size << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

FoldNetworkHistory::FoldNetworkHistory(FoldNetwork* network) {
	this->network = network;

	for (int n_index = 0; n_index < network->flat_size; n_index++) {
		this->flat_input_history.push_back(network->flat_input->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < network->flat_size; n_index++) {
		this->activated_input_history.push_back(network->activated_input->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < network->state_size; n_index++) {
		this->state_input_history.push_back(network->state_input->acti_vals[n_index]);
	}
	this->obs_input_history.push_back(network->obs_input->acti_vals[0]);

	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		this->hidden_history.push_back(network->hidden->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		this->output_history.push_back(network->output->acti_vals[n_index]);
	}
}

void FoldNetworkHistory::reset_weights() {
	FoldNetwork* network = (FoldNetwork*)this->network;
	for (int n_index = 0; n_index < network->flat_size; n_index++) {
		network->flat_input->acti_vals[n_index] = this->flat_input_history[n_index];
	}
	for (int n_index = 0; n_index < network->flat_size; n_index++) {
		network->activated_input->acti_vals[n_index] = this->activated_input_history[n_index];
	}
	for (int n_index = 0; n_index < network->state_size; n_index++) {
		network->state_input->acti_vals[n_index] = this->state_input_history[n_index];
	}
	network->obs_input->acti_vals[0] = this->obs_input_history[0];

	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		network->output->acti_vals[n_index] = this->output_history[n_index];
	}
}
