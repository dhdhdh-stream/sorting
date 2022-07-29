#include "network.h"

using namespace std;

void Network::construct(int input_size,
						int layer_size,
						int output_size) {
	Layer** val_1st_input = new Layer*[1] {
		&this->input
	};
	this->val_1st.setup(RELU_LAYER, 1, val_1st_input);

	Layer** val_val_input = new Layer*[1] {
		&this->val_1st
	};
	this->val_val.setup(LINEAR_LAYER, 1, val_val_input);

	for (int i = 0; i < input_size; i++) {
		this->input.add_node();
	}
	for (int i = 0; i < layer_size; i++) {
		this->val_1st.add_node();
	}
	for (int i = 0; i < output_size; i++) {
		this->val_val.add_node();
	}

	this->val_1st.add_weights();
	this->val_val.add_weights();
}

Network::Network(int input_size,
				 int layer_size,
				 int output_size) {
	construct(input_size, layer_size, output_size);
}

Network::Network(int input_size,
				 int layer_size,
				 int output_size,
				 ifstream& input_file) {
	construct(input_size, layer_size, output_size);

	this->val_1st.copy_weights_from(input_file);
	this->val_val.copy_weights_from(input_file);
}

Network::~Network() {
	// pass
}

void Network::activate(vector<double>& vals) {
	for (int i = 0; i < (int)vals.size(); i++) {
		this->input.acti_vals[i] = vals[i];
	}

	this->val_1st.activate();
	this->val_val.activate();
}

void Network::activate(vector<double>& vals,
					   vector<NetworkHistory*>& network_historys) {
	activate(vals);

	NetworkHistory* network_history = new NetworkHistory(this);
	network_historys.push_back(network_history);
}

void Network::backprop(vector<double>& errors) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->val_val.errors[e_index] = errors[e_index];
	}

	this->val_val.backprop();
	this->val_1st.backprop();
}

void Network::calc_max_update(double& max_update,
							  double learning_rate,
							  double momentum) {
	this->val_1st.calc_max_update(max_update,
								  learning_rate,
								  momentum);
	this->val_val.calc_max_update(max_update,
								  learning_rate,
								  momentum);
}

void Network::update_weights(double factor,
							 double learning_rate,
							 double momentum) {
	this->val_1st.update_weights(factor,
								 learning_rate,
								 momentum);
	this->val_val.update_weights(factor,
								 learning_rate,
								 momentum);
}

void Network::save_weights(ofstream& output_file) {
	this->val_1st.save_weights(output_file);
	this->val_val.save_weights(output_file);
}

NetworkHistory::NetworkHistory(Network* network) {
	this->network = network;

	for (int n_index = 0; n_index < (int)network->input.acti_vals.size(); n_index++) {
		this->input_history.push_back(network->input.acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < (int)network->val_1st.acti_vals.size(); n_index++) {
		this->val_1st_history.push_back(network->val_1st.acti_vals[n_index]);
	}
	for (int n_index = 0; n_index < (int)network->val_val.acti_vals.size(); n_index++) {
		this->val_val_history.push_back(network->val_val.acti_vals[n_index]);
	}
}

NetworkHistory::~NetworkHistory() {
	// pass
}

void NetworkHistory::reset_weights() {
	for (int n_index = 0; n_index < (int)this->network->input.acti_vals.size(); n_index++) {
		this->network->input.acti_vals[n_index] = this->input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->network->val_1st.acti_vals.size(); n_index++) {
		this->network->val_1st.acti_vals[n_index] = this->val_1st_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->network->val_val.acti_vals.size(); n_index++) {
		this->network->val_val.acti_vals[n_index] = this->val_val_history[n_index];
	}
}
