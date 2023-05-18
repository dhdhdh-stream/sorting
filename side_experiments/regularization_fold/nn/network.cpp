#include "network.h"

#include <iostream>

using namespace std;

void Network::construct() {
	this->obs_input = new Layer(LINEAR_LAYER, this->obs_size);
	this->state_input = new Layer(LINEAR_LAYER, this->state_size);

	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
	this->hidden->input_layers.push_back(this->input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::Network(int obs_size,
				 int state_size,
				 int hidden_size) {
	this->obs_size = obs_size;
	this->state_size = state_size;
	this->hidden_size = hidden_size;

	construct();
}

Network::Network(Network* original) {
	this->obs_size = original->obs_size;
	this->state_size = original->state_size;
	this->hidden_size = original->hidden_size;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

Network::Network(ifstream& input_file) {
	string obs_size_line;
	getline(input_file, obs_size_line);
	this->obs_size = stoi(obs_size_line);

	string state_size_line;
	getline(input_file, state_size_line);
	this->state_size = stoi(state_size_line);

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	this->hidden_size = stoi(hidden_size_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

Network::~Network() {
	delete this->obs_input;
	delete this->state_input;
	delete this->hidden;
	delete this->output;
}

void Network::activate() {
	this->hidden->activate();
	this->output->activate();
}

void Network::activate(NetworkHistory* history) {
	activate();

	history->save_weights();
}

void Network::backprop(double target_max_update) {
	this->output->backprop();
	this->hidden->backprop();

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

void Network::backprop(double target_max_update,
					   NetworkHistory* history) {
	history->reset_weights();

	backprop(target_max_update);
}

void Network::lasso_backprop(double lambda,
							 double target_max_update) {
	this->output->backprop();
	this->hidden->backprop();

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
			this->hidden->lasso_update_weights(lambda,
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

void Network::lasso_backprop(double lambda,
							 double target_max_update,
							 NetworkHistory* history) {
	history->reset_weights();

	lasso_backprop(lambda,
				   target_max_update);
}

void Network::backprop_errors_with_no_weight_change() {
	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();
}

void Network::backprop_errors_with_no_weight_change(NetworkHistory* history) {
	history->reset_weights();

	backprop_errors_with_no_weight_change();
}

void Network::backprop_weights_with_no_error_signal(double target_max_update) {
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

void Network::backprop_weights_with_no_error_signal(double target_max_update,
													NetworkHistory* history) {
	history->reset_weights();

	backprop_weights_with_no_error_signal(target_max_update);
}

void Network::add_state() {
	this->hidden->hidden_add_state();

	this->state_size++;
	this->state_input->acti_vals.push_back(0.0);
	this->state_input->errors.push_back(0.0);
}

void Network::calc_state_impact(int index) {
	double impact = 0.0;
	for (int n_index = 0; n_index < this->hidden_size; n_index++) {
		impact += abs(this->hidden->weights[n_index][0][index]);
	}
	return impact;
}

void Network::remove_state(int index) {
	this->hidden->hidden_remove_state(index);

	this->state_size--;
	this->state_input->acti_vals.pop_back();
	this->state_input->errors.pop_back();
}

void Network::save(ofstream& output_file) {
	output_file << this->input_size << endl;
	output_file << this->hidden_size << endl;
	output_file << this->output_size << endl;
	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

NetworkHistory::NetworkHistory(Network* network) {
	this->network = network;
}

void NetworkHistory::save_weights() {
	this->input_history.reserve(network->input->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->input->acti_vals.size(); n_index++) {
		this->input_history.push_back(network->input->acti_vals[n_index]);
	}
	this->hidden_history.reserve(network->hidden->acti_vals.size());
	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		this->hidden_history.push_back(network->hidden->acti_vals[n_index]);
	}
}

void NetworkHistory::reset_weights() {
	for (int n_index = 0; n_index < (int)this->network->input->acti_vals.size(); n_index++) {
		this->network->input->acti_vals[n_index] = this->input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->network->hidden->acti_vals.size(); n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
}
