#include "network.h"

#include <iostream>

#include "globals.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

Network::Network(int unroll_iters,
				 int num_obs,
				 int num_actions) {
	this->unroll_iters = unroll_iters;
	this->num_obs = num_obs;
	this->num_actions = num_actions;

	this->input = new Layer(LINEAR_LAYER);
	int num_inputs = this->unroll_iters * (this->num_obs + this->num_actions);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 16; h_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 8; h_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 4; h_index++) {
		this->hidden_3->acti_vals.push_back(0.0);
		this->hidden_3->errors.push_back(0.0);
	}
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->input);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->hidden_3_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::~Network() {
	delete this->input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->hidden_3;
	delete this->output;
}

void Network::activate(vector<vector<double>>& obs_vals,
					   vector<int>& moves) {
	int input_index = 0;
	for (int iter_index = 0; iter_index < this->unroll_iters; iter_index++) {
		for (int o_index = 0; o_index < this->num_obs; o_index++) {
			this->input->acti_vals[input_index] = obs_vals[iter_index][o_index];
			input_index++;
		}
	}
	for (int iter_index = 0; iter_index < this->unroll_iters; iter_index++) {
		for (int a_index = 0; a_index < this->num_actions; a_index++) {
			if (moves[iter_index] == a_index) {
				this->input->acti_vals[input_index] = 1.0;
				input_index++;
			} else {
				this->input->acti_vals[input_index] = 0.0;
				input_index++;
			}
		}
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();
}

void Network::activate(vector<vector<double>>& obs_vals,
					   vector<int>& moves,
					   NetworkHistory* history) {
	int input_index = 0;
	for (int iter_index = 0; iter_index < this->unroll_iters; iter_index++) {
		for (int o_index = 0; o_index < this->num_obs; o_index++) {
			this->input->acti_vals[input_index] = obs_vals[iter_index][o_index];
			input_index++;
		}
	}
	for (int iter_index = 0; iter_index < this->unroll_iters; iter_index++) {
		for (int a_index = 0; a_index < this->num_actions; a_index++) {
			if (moves[iter_index] == a_index) {
				this->input->acti_vals[input_index] = 1.0;
				input_index++;
			} else {
				this->input->acti_vals[input_index] = 0.0;
				input_index++;
			}
		}
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();

	history->input_history = vector<double>(this->input->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->input->acti_vals.size(); n_index++) {
		history->input_history[n_index] = this->input->acti_vals[n_index];
	}
	history->hidden_1_history = vector<double>(this->hidden_1->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->hidden_1->acti_vals.size(); n_index++) {
		history->hidden_1_history[n_index] = this->hidden_1->acti_vals[n_index];
	}
	history->hidden_2_history = vector<double>(this->hidden_2->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->hidden_2->acti_vals.size(); n_index++) {
		history->hidden_2_history[n_index] = this->hidden_2->acti_vals[n_index];
	}
	history->hidden_3_history = vector<double>(this->hidden_3->acti_vals.size());
	for (int n_index = 0; n_index < (int)this->hidden_3->acti_vals.size(); n_index++) {
		history->hidden_3_history[n_index] = this->hidden_3->acti_vals[n_index];
	}
}

void Network::backprop(double error,
					   NetworkHistory* history) {
	for (int n_index = 0; n_index < (int)this->input->acti_vals.size(); n_index++) {
		this->input->acti_vals[n_index] = history->input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->hidden_1->acti_vals.size(); n_index++) {
		this->hidden_1->acti_vals[n_index] = history->hidden_1_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->hidden_2->acti_vals.size(); n_index++) {
		this->hidden_2->acti_vals[n_index] = history->hidden_2_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->hidden_3->acti_vals.size(); n_index++) {
		this->hidden_3->acti_vals[n_index] = history->hidden_3_history[n_index];
	}

	this->output->errors[0] = error;
	this->output->backprop();
	this->hidden_3->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		double hidden_1_max_update = 0.0;
		this->hidden_1->get_max_update(hidden_1_max_update);
		this->hidden_1_average_max_update = 0.999*this->hidden_1_average_max_update+0.001*hidden_1_max_update;
		if (hidden_1_max_update > 0.0) {
			double hidden_1_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_1_average_max_update;
			if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_1_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_1_max_update;
			}
			this->hidden_1->update_weights(hidden_1_learning_rate);
		}

		double hidden_2_max_update = 0.0;
		this->hidden_2->get_max_update(hidden_2_max_update);
		this->hidden_2_average_max_update = 0.999*this->hidden_2_average_max_update+0.001*hidden_2_max_update;
		if (hidden_2_max_update > 0.0) {
			double hidden_2_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_2_average_max_update;
			if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_2_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_2_max_update;
			}
			this->hidden_2->update_weights(hidden_2_learning_rate);
		}

		double hidden_3_max_update = 0.0;
		this->hidden_3->get_max_update(hidden_3_max_update);
		this->hidden_3_average_max_update = 0.999*this->hidden_3_average_max_update+0.001*hidden_3_max_update;
		if (hidden_3_max_update > 0.0) {
			double hidden_3_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_3_average_max_update;
			if (hidden_3_learning_rate*hidden_3_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_3_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_3_max_update;
			}
			this->hidden_3->update_weights(hidden_3_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > NETWORK_TARGET_MAX_UPDATE) {
				output_learning_rate = NETWORK_TARGET_MAX_UPDATE/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}
