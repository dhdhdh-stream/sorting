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
					   vector<NetworkHistory*>& network_historys) {
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

void Network::add_potential() {
	Layer* new_potential_input = new Layer(LINEAR_LAYER, 1);
	new_potential_input.is_on = false;
	Layer* new_potential_hidden = new Layer(RELU_LAYER, 4);
	new_potential_hidden.is_on = false;
	new_potential_hidden->input_layers.push_back(this->input);
	new_potential_hidden->input_layers.push_back(new_potential_input);
	new_potential_hidden->setup_weights_full();
	this->potential_inputs.push_back(new_potential_input);
	this->potential_hidden.push_back(new_potential_hiddens);

	this->hidden->add_potential_input_layer(new_potential_input);
	this->output->add_potential_input_layer(new_potential_hidden);
}

void Network::activate(vector<double>& vals,
					   vector<int>& potentials_on,
					   vector<double>& potential_vals) {
	for (int i = 0; i < (int)vals.size(); i++) {
		this->input->acti_vals[i] = vals[i];
	}

	for (int p_index = 0; p_index < (int)potentials_on.size(); p_index++) {
		this->potential_inputs[potentials_on[p_index]]->is_on = true;
		this->potential_hiddens[potentials_on[p_index]]->is_on = true;

		this->potential_inputs[potentials_on[p_index]]->acti_vals[0] = potential_vals[p_index];
	}

	this->hidden->activate();
	for (int p_index = 0; p_index < (int)potentials_on.size(); p_index++) {
		this->potential_hiddens[potentials_on[p_index]]->activate();
	}

	this->output->activate();

	for (int p_index = 0; p_index < (int)potentials_on.size(); p_index++) {
		this->potential_inputs[potentials_on[p_index]]->is_on = false;
		this->potential_hiddens[potentials_on[p_index]]->is_on = false;
	}
}

void Network::activate(vector<double>& vals,
					   vector<int>& potentials_on,
					   vector<double>& potential_vals,
					   vector<NetworkHistory*>& network_historys) {
	activate(vals,
			 potentials_on,
			 potential_vals);

	NetworkHistory* network_history = new NetworkHistory(this, potentials_on);
	network_historys.push_back(network_history);
}

void Network::backprop(vector<double>& errors,
					   vector<int>& potentials_on) {
	for (int e_index = 0; e_index < (int)errors.size(); e_index++) {
		this->output->errors[e_index] = errors[e_index];
	}

	for (int p_index = 0; p_index < (int)potentials_on.size(); p_index++) {
		this->potential_inputs[potentials_on[p_index]]->is_on = true;
		this->potential_hiddens[potentials_on[p_index]]->is_on = true;
	}

	this->output->backprop();

	for (int p_index = 0; p_index < (int)potentials_on.size(); p_index++) {
		this->potential_hiddens[potentials_on[p_index]]->backprop();
	}
	this->hidden->backprop();

	for (int p_index = 0; p_index < (int)potentials_on.size(); p_index++) {
		this->potential_inputs[potentials_on[p_index]]->is_on = false;
		this->potential_hiddens[potentials_on[p_index]]->is_on = false;
	}

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

void Network::extend_with_potential(int potential_index) {
	this->input->input_extend_with_potential();
	this->hidden->hidden_extend_with_potential(potential_index,
											   this->potential_hiddens[potential_index]);
	this->output->output_extend_with_potential(potential_index);
}

void Network::reset_potential(int potential_index) {
	this->hidden->reset_potential_input_layer(potential_index);
	this->output->reset_potential_input_layer(potential_index);

	this->potential_hiddens[potential_index]->reset_weights();
}

void Network::reset() {
	this->hidden->reset_weights();
	this->output->reset_weights();
}

void Network::remove_potentials() {
	this->hidden->remove_potential_input_layers();
	this->output->remove_potential_input_layers();

	for (int p_index = 0; p_index < (int)this->potential_hidden.size(); p_index++) {
		delete this->potential_inputs[p_index];
		delete this->potential_hidden[p_index];
	}
	this->potential_inputs.clear();
	this->potential_hidden.clear();
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

NetworkHistory::NetworkHistory(Network* network,
							   vector<int> potentials_on) {
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

	this->potentials_on = potentials_on;
	for (int p_index = 0; p_index < (int)this->potentials_on.size(); p_index++) {
		this->potential_inputs_historys.push_back(network->potential_inputs[this->potentials_on[p_index]]->acti_vals[0]);
		
		vector<double> hidden_history;
		for (int n_index = 0; n_index < (int)network->potential_hiddens[this->potentials_on[p_index]]->acti_vals.size(); n_index++) {
			hidden_history.push_back(network->potential_hiddens[this->potentials_on[p_index]]->acti_vals[n_index]);
		}
		this->potential_hiddens_historys.push_back(hidden_history);
	}
}

NetworkHistory::~NetworkHistory() {
	// pass
}

void NetworkHistory::reset_weights() {
	for (int n_index = 0; n_index < (int)this->network->input->acti_vals.size(); n_index++) {
		this->network->input->acti_vals[n_index] = this->input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->network->hidden->acti_vals.size(); n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->network->output->acti_vals.size(); n_index++) {
		this->network->output->acti_vals[n_index] = this->output_history[n_index];
	}

	for (int p_index = 0; p_index < (int)this->potentials_on.size(); p_index++) {
		this->network->potential_inputs[this->potentials_on[p_index]]->acti_vals[0] = \
			this->potential_inputs_historys[p_index];

		for (int n_index = 0; n_index < (int)this->network->potential_hiddens[this->potentials_on[p_index]]->acti_vals.size(); n_index++) {
			this->network->potential_hiddens[this->potentials_on[p_index]]->acti_vals[n_index] = \
				this->potential_hiddens_historys[p_index][n_index];
		}
	}
}
