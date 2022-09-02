#include "node.h"

using namespace std;

Node::Node(int state_size) {
	this->state_size = state_size;
	
	int input_size = 1 + this->state_size;
	this->network = new Network(input_size, 8*input_size, this->state_size);
}

Node::~Node() {
	delete this->network;
}

void Node::activate(double obs,
				    double* state_vals) {
	vector<double> inputs;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		inputs.push_back(state_vals[s_index]);
	}
	inputs.push_back(obs);

	this->network->activate(inputs);

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_vals[s_index] = this->network->output->acti_vals[s_index];
	}
}

void Node::activate(double obs,
				    double* state_vals,
				    std::vector<NetworkHistory*>& network_historys) {
	vector<double> inputs;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		inputs.push_back(state_vals[s_index]);
	}
	inputs.push_back(obs);

	this->network->activate(inputs, network_historys);

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_vals[s_index] = this->network->output->acti_vals[s_index];
	}
}

void Node::activate_greedy(double obs,
						   double* state_vals) {
	vector<double> inputs;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		inputs.push_back(state_vals[s_index]);
	}
	inputs.push_back(obs);

	this->network->activate(inputs);

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_vals[s_index] += this->network->output->acti_vals[s_index];
	}
}

void Node::activate_greedy(double obs,
						   double* state_vals,
						   std::vector<NetworkHistory*>& network_historys) {
	vector<double> inputs;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		inputs.push_back(state_vals[s_index]);
	}
	inputs.push_back(obs);

	this->network->activate(inputs, network_historys);

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_vals[s_index] += this->network->output->acti_vals[s_index];
	}
}

void Node::backprop(double* state_errors,
					std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	network_history->reset_weights();

	vector<double> errors;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		errors.push_back(state_errors[s_index]);
	}

	this->network->backprop(errors);

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_errors[s_index] = this->network->input->errors[s_index];
		this->network->input->errors[s_index] = 0.0;
	}

	delete network_history;
	network_historys.pop_back();
}

void Node::backprop_errors_with_no_weight_change(
		double* state_errors,
		std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	network_history->reset_weights();

	vector<double> errors;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		errors.push_back(state_errors[s_index]);
	}

	this->network->backprop_errors_with_no_weight_change(errors);

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_errors[s_index] = this->network->input->errors[s_index];
		this->network->input->errors[s_index] = 0.0;
	}

	delete network_history;
	network_historys.pop_back();
}

void Node::backprop_greedy(double* state_errors,
						   std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	network_history->reset_weights();

	vector<double> errors;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		errors.push_back(state_errors[s_index]);
	}

	this->network->backprop(errors);

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_errors[s_index] += this->network->input->errors[s_index];
		this->network->input->errors[s_index] = 0.0;
	}

	delete network_history;
	network_historys.pop_back();
}

void Node::backprop_greedy_errors_with_no_weight_change(
		double* state_errors,
		std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	network_history->reset_weights();

	vector<double> errors;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		errors.push_back(state_errors[s_index]);
	}

	this->network->backprop_errors_with_no_weight_change(errors);

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_errors[s_index] += this->network->input->errors[s_index];
		this->network->input->errors[s_index] = 0.0;
	}

	delete network_history;
	network_historys.pop_back();
}
