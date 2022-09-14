#include "network.h"

#include <iostream>

using namespace std;

void Network::construct(int input_size,
						int hidden_size,
						int output_size) {
	this->input = new Layer(LINEAR_LAYER, input_size);
	
	this->hidden = new Layer(RELU_LAYER, hidden_size);
	this->hidden->input_layers.push_back(this->input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, output_size);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();
}

Network::Network(int input_size,
				 int hidden_size,
				 int output_size) {
	construct(input_size, hidden_size, output_size);

	this->epoch = 0;
	this->iter = 0;
}

Network::Network(Network* original) {
	construct(
		(int)original->input->acti_vals.size(),
		(int)original->hidden->acti_vals.size(),
		(int)original->output->acti_vals.size());

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);

	this->epoch = 0;
	this->iter = 0;
}

Network::Network(ifstream& input_file) {
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	int hidden_size = stoi(hidden_size_line);

	string output_size_line;
	getline(input_file, output_size_line);
	int output_size = stoi(output_size_line);

	construct(input_size, hidden_size, output_size);

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	string epoch_line;
	getline(input_file, epoch_line);
	this->epoch = stoi(epoch_line);
	this->iter = 0;
}

Network::~Network() {
	delete this->input;
	delete this->hidden;
	delete this->output;
}

void Network::activate(vector<double>& vals) {
	for (int i = 0; i < (int)vals.size(); i++) {
		this->input->acti_vals[i] = vals[i];
	}

	this->hidden->activate();
	this->output->activate();
}

void Network::activate(vector<double>& vals,
					   vector<AbstractNetworkHistory*>& network_historys) {
	activate(vals);

	NetworkHistory* network_history = new NetworkHistory(this);
	network_historys.push_back(network_history);
}

void Network::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop();
	this->hidden->backprop();

	if (this->iter == 100) {
		double max_update = 0.0;
		calc_max_update(max_update,
						0.001,
						0.2);
		double factor = 1.0;
		if (max_update > 0.01) {
			factor = 0.01/max_update;
		}
		update_weights(factor,
					   0.001,
					   0.2);

		this->epoch++;
		this->iter = 0;
	} else {
		this->iter++;
	}
}

void Network::calc_max_update(double& max_update,
							  double learning_rate,
							  double momentum) {
	this->hidden->calc_max_update(max_update,
								  learning_rate,
								  momentum);
	this->output->calc_max_update(max_update,
								  learning_rate,
								  momentum);
}

void Network::update_weights(double factor,
							 double learning_rate,
							 double momentum) {
	this->hidden->update_weights(factor,
								 learning_rate,
								 momentum);
	this->output->update_weights(factor,
								 learning_rate,
								 momentum);
}

void Network::backprop_errors_with_no_weight_change(std::vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();
}

void Network::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;
	output_file << this->hidden->acti_vals.size() << endl;
	output_file << this->output->acti_vals.size() << endl;
	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
	output_file << this->epoch << endl;
}

NetworkHistory::NetworkHistory(Network* network) {
	this->network = network;

	for (int n_index = 0; n_index < (int)network->input->acti_vals.size(); n_index++) {
		this->input_history.push_back(network->input->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		this->hidden_history.push_back(network->hidden->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		this->output_history.push_back(network->output->acti_vals[n_index]);
	}
}

void NetworkHistory::reset_weights() {
	Network* network = (Network*)this->network;
	for (int n_index = 0; n_index < (int)network->input->acti_vals.size(); n_index++) {
		network->input->acti_vals[n_index] = this->input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->hidden->acti_vals.size(); n_index++) {
		network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
	for (int n_index = 0; n_index < (int)network->output->acti_vals.size(); n_index++) {
		network->output->acti_vals[n_index] = this->output_history[n_index];
	}
}
