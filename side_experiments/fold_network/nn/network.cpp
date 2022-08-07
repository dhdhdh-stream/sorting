#include "network.h"

#include <iostream>

using namespace std;

void Network::construct(int input_size,
						int layer_size,
						int output_size) {
	this->input = new Layer(LINEAR_LAYER, input_size);
	
	this->val_1st = new Layer(RELU_LAYER, layer_size);
	this->val_1st->input_layers.push_back(this->input);
	this->val_1st->setup_weights_full();

	this->val_val = new Layer(LINEAR_LAYER, output_size);
	this->val_val->input_layers.push_back(this->val_1st);
	this->val_val->setup_weights_full();
}

Network::Network(int input_size,
				 int layer_size,
				 int output_size) {
	construct(input_size, layer_size, output_size);

	this->epoch = 0;
	this->iter = 0;
}

Network::Network(ifstream& input_file) {
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);

	string layer_size_line;
	getline(input_file, layer_size_line);
	int layer_size = stoi(layer_size_line);

	string output_size_line;
	getline(input_file, output_size_line);
	int output_size = stoi(output_size_line);

	construct(input_size, layer_size, output_size);

	this->val_1st->load_weights_from(input_file);
	this->val_val->load_weights_from(input_file);

	string epoch_line;
	getline(input_file, epoch_line);
	this->epoch = stoi(epoch_line);
	this->iter = 0;
}

Network::~Network() {
	delete this->input;
	delete this->val_1st;
	delete this->val_val;
}

void Network::activate(vector<double>& vals) {
	for (int i = 0; i < (int)vals.size(); i++) {
		this->input->acti_vals[i] = vals[i];
	}

	this->val_1st->activate();
	this->val_val->activate();
}

void Network::activate(vector<double>& vals,
					   vector<NetworkHistory*>& network_historys) {
	activate(vals);

	NetworkHistory* network_history = new NetworkHistory(this);
	network_historys.push_back(network_history);
}

void Network::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->val_val->errors[e_index] = errors[e_index];
	}

	this->val_val->backprop();
	this->val_1st->backprop();
}

void Network::increment() {
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
	this->val_1st->calc_max_update(max_update,
								   learning_rate,
								   momentum);
	this->val_val->calc_max_update(max_update,
								   learning_rate,
								   momentum);
}

void Network::update_weights(double factor,
							 double learning_rate,
							 double momentum) {
	this->val_1st->update_weights(factor,
								  learning_rate,
								  momentum);
	this->val_val->update_weights(factor,
								  learning_rate,
								  momentum);
}

void Network::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;
	output_file << this->val_1st->acti_vals.size() << endl;
	output_file << this->val_val->acti_vals.size() << endl;
	this->val_1st->save_weights(output_file);
	this->val_val->save_weights(output_file);
	output_file << this->epoch << endl;
}

NetworkHistory::NetworkHistory(Network* network) {
	this->network = network;

	for (int n_index = 0; n_index < (int)network->input->acti_vals.size(); n_index++) {
		this->input_history.push_back(network->input->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < (int)network->val_1st->acti_vals.size(); n_index++) {
		this->val_1st_history.push_back(network->val_1st->acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < (int)network->val_val->acti_vals.size(); n_index++) {
		this->val_val_history.push_back(network->val_val->acti_vals[n_index]);
	}
}

NetworkHistory::~NetworkHistory() {
	// pass
}

void NetworkHistory::reset_weights() {
	for (int n_index = 0; n_index < (int)this->network->input->acti_vals.size(); n_index++) {
		this->network->input->acti_vals[n_index] = this->input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->network->val_1st->acti_vals.size(); n_index++) {
		this->network->val_1st->acti_vals[n_index] = this->val_1st_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->network->val_val->acti_vals.size(); n_index++) {
		this->network->val_val->acti_vals[n_index] = this->val_val_history[n_index];
	}
}
