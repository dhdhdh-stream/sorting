#include "flat_network.h"

#include "layer.h"

using namespace std;

/**
 * - can use lower learning rate as not meant to solve problem, but to discover which inputs are relevant
 */
const double FLAT_NETWORK_TARGET_MAX_UPDATE = 0.01;

FlatNetwork::FlatNetwork(int num_inputs) {
	this->num_inputs = num_inputs;
	this->input = new Layer(LINEAR_LAYER, this->num_inputs);

	this->hidden = new Layer(LEAKY_LAYER, FLAT_NETWORK_HIDDEN_SIZE);
	this->hidden->input_layers.push_back(this->input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FlatNetwork::~FlatNetwork() {
	delete this->input;
	delete this->hidden;
	delete this->output;
}

void FlatNetwork::activate() {
	this->hidden->activate();
	this->output->activate();
}

void FlatNetwork::backprop(double error) {
	this->output->errors[0] = error;

	this->output->backprop();
	this->hidden->backprop();

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
