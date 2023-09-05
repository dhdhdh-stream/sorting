#include "state_network.h"

using namespace std;

void StateNetwork::construct() {
	this->obs_input = new Layer(LINEAR_LAYER, 1);
	this->state_input = new Layer(LINEAR_LAYER, 1);

	this->hidden = new Layer(LEAKY_LAYER, STATE_NETWORK_HIDDEN_SIZE);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

StateNetwork::StateNetwork() {
	this->ending_state_mean = 0.0;
	this->ending_state_variance = 1.0;
	this->ending_state_standard_deviation = 1.0;

	construct();
}

StateNetwork::StateNetwork(StateNetwork* original) {
	this->ending_state_mean = original->ending_state_mean;
	this->ending_state_variance = original->ending_state_variance;
	this->ending_state_standard_deviation = original->ending_state_standard_deviation;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

StateNetwork::StateNetwork(ifstream& input_file) {
	string ending_state_mean_line;
	getline(input_file, ending_state_mean_line);
	this->ending_state_mean = stod(ending_state_mean_line);

	string ending_state_variance_line;
	getline(input_file, ending_state_variance_line);
	this->ending_state_variance = stod(ending_state_variance_line);

	string ending_state_standard_deviation_line;
	getline(input_file, ending_state_standard_deviation_line);
	this->ending_state_standard_deviation = stod(ending_state_standard_deviation_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

StateNetwork::~StateNetwork() {
	delete this->obs_input;
	delete this->state_input;
	delete this->hidden;
	delete this->output;
}

void StateNetwork::activate(double obs_val,
							double& state_val) {
	this->obs_input->acti_vals[0] = obs_val;
	this->state_input->acti_vals[0] = state_val;

	this->hidden->activate();
	this->output->activate();

	state_val += this->output->acti_vals[0];

	state_val = (state_val-this->ending_state_mean)/this->ending_state_standard_deviation;
}

void StateNetwork::activate(double obs_val,
							double& state_val,
							StateNetworkHistory* history) {
	activate(obs_val,
			 state_val);

	history->save_weights();
}

void StateNetwork::backprop(double& state_error,
							double target_max_update) {
	this->output->errors[0] = state_error/this->ending_state_standard_deviation;

	this->output->backprop();
	this->hidden->state_hidden_backprop();

	state_error += this->state_input->errors[0];
	this->state_input->errors[0] = 0.0;

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

void StateNetwork::backprop(double& state_error,
							double target_max_update,
							double obs_snapshot,
							StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	history->reset_weights();

	backprop(state_error,
			 target_max_update);
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double& state_error) {
	this->output->errors[0] = state_error/this->ending_state_standard_deviation;

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();

	state_error += this->state_input->errors[0];
	this->state_input->errors[0] = 0.0;
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double& state_error,
		double obs_snapshot,
		StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	history->reset_weights();

	backprop_errors_with_no_weight_change(state_error);
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double& obs_error,
		double& state_error) {
	this->output->errors[0] = state_error/this->ending_state_standard_deviation;

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();

	obs_error += this->obs_input->errors[0];
	this->obs_input->errors[0] = 0.0;

	state_error += this->state_input->errors[0];
	this->state_input->errors[0] = 0.0;
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double& obs_error,
		double& state_error,
		double obs_snapshot,
		StateNetworkHistory* history) {
	this->obs_input->acti_vals[0] = obs_snapshot;
	history->reset_weights();

	backprop_errors_with_no_weight_change(obs_error,
										  state_error);
}

void StateNetwork::save(ofstream& output_file) {
	output_file << this->ending_state_mean << endl;
	output_file << this->ending_state_variance << endl;
	output_file << this->ending_state_standard_deviation << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

StateNetworkHistory::StateNetworkHistory(StateNetwork* network) {
	this->network = network;
}

void StateNetworkHistory::save_weights() {
	this->state_history = this->network->state_input->acti_vals[0];

	this->hidden_history.reserve(STATE_NETWORK_HIDDEN_SIZE);
	for (int n_index = 0; n_index < STATE_NETWORK_HIDDEN_SIZE; n_index++) {
		this->hidden_history.push_back(this->network->hidden->acti_vals[n_index]);
	}
}

void StateNetworkHistory::reset_weights() {
	this->network->state_input->acti_vals[0] = this->state_history;

	for (int n_index = 0; n_index < STATE_NETWORK_HIDDEN_SIZE; n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
}
