#include "update_size_network.h"

using namespace std;

const int HIDDEN_SIZE = 10;

const double TARGET_MAX_UPDATE = 0.005;

void UpdateSizeNetwork::construct() {
	this->obs_input = new Layer(LINEAR_LAYER, 1);
	this->index_input = new Layer(LINEAR_LAYER, 1);

	this->hidden = new Layer(LEAKY_LAYER, HIDDEN_SIZE);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->input_layers.push_back(this->index_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(SIGMOID_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

UpdateSizeNetwork::UpdateSizeNetwork() {
	construct();

	this->output->constants[0] = 10.0;
}

UpdateSizeNetwork::UpdateSizeNetwork(ifstream& input_file) {
	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

UpdateSizeNetwork::~UpdateSizeNetwork() {
	delete this->obs_input;
	delete this->index_input;
	delete this->hidden;
	delete this->output;
}

void UpdateSizeNetwork::activate(double obs_val,
								 double index_val,
								 UpdateSizeNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_val;
	this->index_input->acti_vals[0] = index_val;

	this->hidden->activate();
	this->output->activate();

	history->save_weights();
}

void UpdateSizeNetwork::backprop(double error,
								 UpdateSizeNetworkHistory* history) {
	history->reset_weights();

	this->output->errors[0] = error;

	this->output->backprop();
	this->hidden->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == 20) {
		double hidden_max_update = 0.0;
		this->hidden->get_max_update(hidden_max_update);
		this->hidden_average_max_update = 0.999*this->hidden_average_max_update+0.001*hidden_max_update;
		if (hidden_max_update > 0.0) {
			double hidden_learning_rate = (0.3*TARGET_MAX_UPDATE)/this->hidden_average_max_update;
			if (hidden_learning_rate*hidden_max_update > TARGET_MAX_UPDATE) {
				hidden_learning_rate = TARGET_MAX_UPDATE/hidden_max_update;
			}
			this->hidden->update_weights(hidden_learning_rate);
		}

		double output_max_update = 0.0;
		this->output->get_max_update(output_max_update);
		this->output_average_max_update = 0.999*this->output_average_max_update+0.001*output_max_update;
		if (output_max_update > 0.0) {
			double output_learning_rate = (0.3*TARGET_MAX_UPDATE)/this->output_average_max_update;
			if (output_learning_rate*output_max_update > TARGET_MAX_UPDATE) {
				output_learning_rate = TARGET_MAX_UPDATE/output_max_update;
			}
			this->output->update_weights(output_learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void UpdateSizeNetwork::activate(double obs_val,
								 double index_val) {
	this->obs_input->acti_vals[0] = obs_val;
	this->index_input->acti_vals[0] = index_val;

	this->hidden->activate();
	this->output->activate();
}

void UpdateSizeNetwork::save(ofstream& output_file) {
	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

UpdateSizeNetworkHistory::UpdateSizeNetworkHistory(UpdateSizeNetwork* network) {
	this->network = network;
}

void UpdateSizeNetworkHistory::save_weights() {
	this->obs_input_history = network->obs_input->acti_vals[0];
	this->index_input_history = network->index_input->acti_vals[0];

	this->hidden_history.reserve(HIDDEN_SIZE);
	for (int n_index = 0; n_index < HIDDEN_SIZE; n_index++) {
		this->hidden_history.push_back(network->hidden->acti_vals[n_index]);
	}
}

void UpdateSizeNetworkHistory::reset_weights() {
	network->obs_input->acti_vals[0] = this->obs_input_history;
	network->index_input->acti_vals[0] = this->index_input_history;

	for (int n_index = 0; n_index < HIDDEN_SIZE; n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
}
