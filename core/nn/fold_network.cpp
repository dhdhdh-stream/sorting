#include "fold_network.h"

using namespace std;

FoldNetwork::FoldNetwork(int flat_size,
						 int output_size) {
	this->flat_size = flat_size;
	this->output_size = output_size;

	this->flat_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->activated_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->state_input = NULL;
	this->obs_input = new Layer(LINEAR_LAYER, 1);

	int total_input_size = 2*this->flat_size + 2 + 1;
	this->hidden = new Layer(RELU_LAYER, total_input_size*total_input_size);
	this->hidden->input_layers.push_back(this->flat_input);
	this->hidden->input_layers.push_back(this->activated_input);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch = 0;
	this->iter = 0;
}

FoldNetwork::FoldNetwork(int flat_size,
						 int state_size,
						 int output_size) {
	this->flat_size = flat_size;
	this->output_size = output_size;

	this->state_size = state_size;

	this->flat_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->activated_input = new Layer(LINEAR_LAYER, this->flat_size);
	this->state_input = new Layer(LINEAR_LAYER, this->state_size);
	this->obs_input = new Layer(LINEAR_LAYER, 1);

	int total_input_size = 2*this->flat_size + 2 + 1;
	this->hidden = new Layer(RELU_LAYER, total_input_size*total_input_size);
	this->hidden->input_layers.push_back(this->flat_input);
	this->hidden->input_layers.push_back(this->activated_input);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch = 0;
	this->iter = 0;
}

FoldNetwork::~FoldNetwork() {
	delete this->flat_input;
	delete this->activated_input;
	if (this->state_input != NULL) {
		delete this->state_input;
	}
	delete this->obs_input;

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

	this->hidden->activate();
	this->output->activate();

	FoldNetworkHistory* network_history = new FoldNetworkHistory(this);
	network_historys.push_back(network_history);
}

void FoldNetwork::activate(double* flat_inputs,
						   bool* activated,
						   vector<double>& state_vals,
						   vector<double>& obs) {
	for (int f_index = 0; f_index < this->flat_size; f_index++) {
		this->flat_input->acti_vals[f_index] = flat_inputs[f_index];

		if (activated[f_index]) {
			this->activated_input->acti_vals[f_index] = 1.0;
		} else {
			this->activated_input->acti_vals[f_index] = 0.0;
		}
	}

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}

	this->obs_input->acti_vals[0] = obs[0];

	this->hidden->activate();
	this->output->activate();
}

void FoldNetwork::activate(double* flat_inputs,
						   bool* activated,
						   vector<double>& state_vals,
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

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}

	this->obs_input->acti_vals[0] = obs[0];

	this->hidden->activate();
	this->output->activate();

	FoldNetworkHistory* network_history = new FoldNetworkHistory(this);
	network_historys.push_back(network_history);
}

void FoldNetwork::full_backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();

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

void FoldNetwork::set_state_size(int state_size) {
	this->state_size = state_size;
	this->state_network = new Layer(LINEAR_LAYER, state_size);

	this->hidden->insert_input_layer(2, this->state_network);
}

void FoldNetwork::state_backprop(std::vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop_fold_state();

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

FoldNetworkHistory::FoldNetworkHistory(FoldNetwork* network) {
	this->network = network;

	for (int n_index = 0; n_index < network->flat_size; n_index++) {
		this->flat_input_history.push_back(network->flat_input->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < network->flat_size; n_index++) {
		this->activated_input_history.push_back(network->activated_input->acti_vals[n_index]);
	}
	if (network->state_input != NULL) {
		for (int n_index = 0; n_index < network->state_size; n_index++) {
			this->state_input_history.push_back(network->state_input->acti_vals[n_index]);
		}
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
	for (int n_index = 0; n_index < this->network->flat_size; n_index++) {
		this->network->flat_input->acti_vals[n_index] = this->flat_input_history[n_index];
	}
	for (int n_index = 0; n_index < this->network->flat_size; n_index++) {
		this->network->activated_input->acti_vals[n_index] = this->activated_input_history[n_index];
	}
	if (this->network->state_input != NULL) {
		for (int n_index = 0; n_index < this->network->state_size; n_index++) {
			this->network->state_input->acti_vals[n_index] = this->state_input_history[n_index];
		}
	}
	this->network->obs_input->acti_vals[0] = this->obs_input_history[0];

	for (int n_index = 0; n_index < (int)this->network->hidden->acti_vals.size(); n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->network->output->acti_vals.size(); n_index++) {
		this->network->output->acti_vals[n_index] = this->output_history[n_index];
	}
}
