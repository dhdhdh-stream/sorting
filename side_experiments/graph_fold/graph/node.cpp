#include "node.h"

using namespace std;

Node::Node(int state_size) {
	this->network_on = false;
	this->network = new Network(3, 24, 2);
}

Node::~Node() {
	delete this->network;
}

void Node::activate(double obs,
				    double* state_vals,
				    std::vector<NetworkHistory*>& network_historys) {
	vector<double> inputs;
	inputs.push_back(state_vals[0]);
	inputs.push_back(state_vals[1]);
	inputs.push_back(obs);

	this->network->activate(inputs, network_historys);

	state_vals[0] = this->network->output->acti_vals[0];
	state_vals[1] = this->network->output->acti_vals[1];
}

void Node::backprop(double* state_errors,
					std::vector<NetworkHistory*>& network_historys) {
	NetworkHistory* network_history = network_historys.back();

	network_history->reset_weights();

	vector<double> errors;
	errors.push_back(state_errors[0]);
	errors.push_back(state_errors[1]);

	this->network->backprop(errors);

	state_errors[0] = this->network->input->errors[0];
	this->network->input->errors[0] = 0.0;
	state_errors[1] = this->network->input->errors[1];
	this->network->input->errors[1] = 0.0;

	delete network_history;
	network_historys.pop_back();
}
