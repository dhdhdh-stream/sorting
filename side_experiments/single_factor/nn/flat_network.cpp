#include "flat_network.h"

using namespace std;

const int FLAT_NETWORK_HIDDEN_SIZE = 20;	// enough to solve 4-way XORs

const int FLAT_NETWORK_TARGET_MAX_UPDATE = 0.05;

FlatNetwork::FlatNetwork() {
	for (int l_index = 0; l_index < FLAT_NETWORK_OBS_SIZE; l_index++) {
		this->obs_inputs.push_back(new Layer(LINEAR_LAYER, 1));
	}

	this->hidden = new Layer(LEAKY_LAYER, FLAT_NETWORK_HIDDEN_SIZE);
	for (int l_index = 0; l_index < FLAT_NETWORK_OBS_SIZE; l_index++) {
		this->hidden->input_layers.push_back(this->obs_inputs[l_index]);
	}
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FlatNetwork::~FlatNetwork() {
	for (int l_index = 0; l_index < FLAT_NETWORK_OBS_SIZE; l_index++) {
		delete this->obs_inputs[l_index];
	}
	delete this->hidden;
	delete this->output;
}

void FlatNetwork::activate(vector<vector<double>>& obs_vals) {
	for (int l_index = 0; l_index < FLAT_NETWORK_OBS_SIZE; l_index++) {
		if (obs_vals[l_index].size() > this->obs_inputs[l_index]->acti_vals.size()) {
			int size_diff = obs_vals[l_index].size() - this->obs_inputs[l_index]->acti_vals.size();
			for (int s_index = 0; s_index < size_diff; s_index++) {
				this->obs_inputs[l_index]->acti_vals.push_back(0.0);
				this->obs_inputs[l_index]->errors.push_back(0.0);

				for (int n_index = 0; n_index < FLAT_NETWORK_HIDDEN_SIZE; n_index++) {
					this->hidden->weights[n_index][1+l_index].push_back((randuni()-0.5)*0.02);
					this->hidden->weight_updates[n_index][1+l_index].push_back(0.0);
				}
			}
		}

		for (int i_index = 0; i_index < obs_vals[l_index].size(); i_index++) {
			this->obs_inputs[l_index][i_index] = obs_vals[l_index][i_index];
		}
		for (int i_index = obs_vals[l_index].size(); i_index < this->obs_inputs[l_index]->acti_vals.size(); i_index++) {
			this->obs_inputs[l_index][i_index] = 0.0;
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void FlatNetwork::backprop(double output_error) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*FLAT_NETWORK_TARGET_MAX_UPDATE)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > FLAT_NETWORK_TARGET_MAX_UPDATE) {
				hidden_learning_rate = FLAT_NETWORK_TARGET_MAX_UPDATE/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*FLAT_NETWORK_TARGET_MAX_UPDATE)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > FLAT_NETWORK_TARGET_MAX_UPDATE) {
				output_learning_rate = FLAT_NETWORK_TARGET_MAX_UPDATE/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}
