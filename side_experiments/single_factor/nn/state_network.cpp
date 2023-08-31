#include "state_network.h"

using namespace std;

void StateNetwork::construct() {
	this->obs_input = new Layer(LINEAR_LAYER, this->obs_size);
	this->state_input = new Layer(LINEAR_LAYER, 1);

	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
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

StateNetwork::StateNetwork(int obs_size,
						   int hidden_size) {
	this->obs_size = obs_size;
	this->hidden_size = hidden_size;

	this->obs_weight = 0.5;

	this->ending_state_mean = 0.0;
	this->ending_state_variance = 1.0;

	construct();
}

StateNetwork::StateNetwork(StateNetwork* original) {
	this->obs_size = original->obs_size;
	this->hidden_size = original->hidden_size;

	

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

StateNetwork::StateNetwork(ifstream& input_file) {
	string obs_size_line;
	getline(input_file, obs_size_line);
	this->obs_size = stoi(obs_size_line);

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	this->hidden_size = stoi(hidden_size_line);

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

void StateNetwork::activate(std::vector<double>& obs_vals,
							double& state_val) {
	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = this->obs_scale*obs_vals[o_index];
	}
	this->state_input->acti_vals[0] = state_val;

	this->hidden->activate();
	this->output->activate();

	state_val += this->obs_weight*this->output->acti_vals[0];

	double ending_state_standard_deviation = sqrt(this->ending_state_variance);
	state_val = (state_val-this->ending_state_mean)/ending_state_standard_deviation;
	// HERE
}

void StateNetwork::activate(std::vector<double>& obs_vals,
							double& state_val,
							StateNetworkHistory* history) {
	activate(obs_vals,
			 state_val);

	history->save_weights();
}

void StateNetwork::backprop(double& state_error,
							double target_max_update) {
	this->output->errors[0] = this->obs_weight*state_error;

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

		this->obs_weight = 0.0;
		for (int n_index = 0; n_index < this->hidden_size; n_index++) {
			this->obs_weight += abs(this->hidden->weights[n_index][0][0]
				* this->output->weights[0][0][n_index]);
		}
		this->obs_weight = max(0.01, min(10.0, this->obs_weight));
		this->obs_weight /= 10.0;

		this->epoch_iter = 0;
	}
}

void StateNetwork::backprop(double& state_error,
							double target_max_update,
							vector<double>& obs_history,
							StateNetworkHistory* history) {
	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = this->obs_scale*obs_history[o_index];
	}
	history->reset_weights();

	backprop(state_error,
			 target_max_update);
}

void StateNetwork::lasso_backprop(double& state_error,
								  double target_max_update) {
	this->output->errors[0] = this->obs_weight*state_error;

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
			this->hidden->state_hidden_lasso_update_weights(
				hidden_learning_rate,
				target_max_update);
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

		this->obs_weight = 0.0;
		for (int n_index = 0; n_index < this->hidden_size; n_index++) {
			this->obs_weight += abs(this->hidden->weights[n_index][0][0]
				* this->output->weights[0][0][n_index]);
		}
		this->obs_weight = max(0.01, min(10.0, this->obs_weight));
		this->obs_weight /= 10.0;

		this->epoch_iter = 0;
	}
}

void StateNetwork::lasso_backprop(double& state_error,
								  double target_max_update,
								  vector<double>& obs_history,
								  StateNetworkHistory* history) {
	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = this->obs_scale*obs_history[o_index];
	}
	history->reset_weights();

	lasso_backprop(state_error,
				   target_max_update);
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double& state_error) {
	this->output->errors[0] = this->obs_weight*state_error;

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();

	state_error += this->state_input->errors[0];
	this->state_input->errors[0] = 0.0;
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double& state_error,
		vector<double>& obs_history,
		StateNetworkHistory* history) {
	for (int o_index = 0; o_index < this->obs_size; o_index++) {
		this->obs_input->acti_vals[o_index] = this->obs_scale*obs_history[o_index];
	}
	history->reset_weights();

	backprop_errors_with_no_weight_change(state_error);
}

void StateNetwork::save(ofstream& output_file) {
	output_file << this->obs_size << endl;
	output_file << this->hidden_size << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

StateNetworkHistory::StateNetworkHistory(StateNetwork* network) {
	this->network = network;
}

void StateNetworkHistory::save_weights() {
	this->state_history = this->network->state_input->acti_vals[0];

	this->hidden_history.reserve(this->network->hidden_size);
	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->hidden_history.push_back(this->network->hidden->acti_vals[n_index]);
	}
}

void StateNetworkHistory::reset_weights() {
	this->network->state_input->acti_vals[0] = this->state_history;

	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
}
