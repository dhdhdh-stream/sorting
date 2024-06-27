#include "shallow_duplicate_network.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
// const double NETWORK_TARGET_MAX_UPDATE = 0.002;
const int EPOCH_SIZE = 20;

ShallowDuplicateNetwork::ShallowDuplicateNetwork() {
	this->input = new Layer(LINEAR_LAYER);
	// for (int i_index = 0; i_index < 50; i_index++) {
	// for (int i_index = 0; i_index < 200; i_index++) {
	for (int i_index = 0; i_index < 800; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}
	this->is_on_input = new Layer(LINEAR_LAYER);
	// for (int i_index = 0; i_index < 50; i_index++) {
	// for (int i_index = 0; i_index < 200; i_index++) {
	for (int i_index = 0; i_index < 800; i_index++) {
		this->is_on_input->acti_vals.push_back(0.0);
		this->is_on_input->errors.push_back(0.0);
	}

	this->hidden = new Layer(LEAKY_LAYER);
	this->hidden->input_layers.push_back(this->input);
	this->hidden->input_layers.push_back(this->is_on_input);
	for (int n_index = 0; n_index < 10; n_index++) {
	// for (int n_index = 0; n_index < 4; n_index++) {
		this->hidden->acti_vals.push_back(0.0);
		this->hidden->errors.push_back(0.0);
	}
	this->hidden->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->input_layers.push_back(this->hidden);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

ShallowDuplicateNetwork::~ShallowDuplicateNetwork() {
	delete this->input;
	delete this->is_on_input;
	delete this->hidden;
	delete this->output;
}

void ShallowDuplicateNetwork::activate(vector<double>& input_vals,
									   vector<bool>& is_on) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		if (is_on[i_index]) {
			this->input->acti_vals[i_index] = input_vals[i_index];
			this->is_on_input->acti_vals[i_index] = 1.0;
		} else {
			this->input->acti_vals[i_index] = 0.0;
			this->is_on_input->acti_vals[i_index] = -1.0;
		}
	}
	this->hidden->activate();
	this->output->activate();
}

void ShallowDuplicateNetwork::backprop(double error) {
	this->output->errors[0] = error;
	this->output->backprop();
	this->hidden->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
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
