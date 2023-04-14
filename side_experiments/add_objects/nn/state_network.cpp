#include "state_network.h"

#include <iostream>

using namespace std;

void StateNetwork::construct() {
	this->obs_input = new Layer(LINEAR_LAYER, this->obs_size);
	this->state_input = new Layer(LINEAR_LAYER, this->state_size);
	this->new_inner_state_input = new Layer(LINEAR_LAYER, this->new_inner_state_size);
	this->new_outer_state_input = new Layer(LINEAR_LAYER, this->new_outer_state_size);

	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->input_layers.push_back(this->state_input);
	this->hidden->input_layers.push_back(this->new_inner_state_input);
	this->hidden->input_layers.push_back(this->new_outer_state_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

StateNetwork::StateNetwork(int obs_size,
						   int state_size,
						   int new_inner_state_size,
						   int new_outer_state_size,
						   int hidden_size) {
	this->obs_size = obs_size;
	this->state_size = state_size;
	this->new_inner_state_size = new_inner_state_size;
	this->new_outer_state_size = new_outer_state_size;
	this->hidden_size = hidden_size;

	construct();

	this->state_zeroed = vector<bool>(this->state_size, false);
	this->new_inner_state_zeroed = vector<bool>(this->new_inner_state_size, false);
	this->new_outer_state_zeroed = vector<bool>(this->new_outer_state_size, false);
}

StateNetwork::StateNetwork(StateNetwork* original) {
	this->obs_size = original->obs_size;
	this->state_size = original->state_size;
	this->new_inner_state_size = original->new_inner_state_size;
	this->new_outer_state_size = original->new_outer_state_size;
	this->hidden_size = original->hidden_size;

	construct();

	this->state_zeroed = original->state_zeroed;
	this->new_inner_state_zeroed = original->new_inner_state_zeroed;
	this->new_outer_state_zeroed = original->new_outer_state_zeroed;

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

StateNetwork::StateNetwork(ifstream& input_file) {
	string obs_size_line;
	getline(input_file, obs_size_line);
	this->obs_size = stoi(obs_size_line);

	string state_size_line;
	getline(input_file, state_size_line);
	this->state_size = stoi(state_size_line);

	string new_inner_state_size_line;
	getline(input_file, new_inner_state_size_line);
	this->new_inner_state_size = stoi(new_inner_state_size_line);

	string new_outer_state_size_line;
	getline(input_file, new_outer_state_size_line);
	this->new_outer_state_size = stoi(new_outer_state_size_line);

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	this->hidden_size = stoi(hidden_size_line);

	construct();

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		string zeroed_line;
		getline(input_file, zeroed_line);
		this->state_zeroed.push_back(stoi(zeroed_line));
	}
	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		string zeroed_line;
		getline(input_file, zeroed_line);
		this->new_inner_state_zeroed.push_back(stoi(zeroed_line));
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		string zeroed_line;
		getline(input_file, zeroed_line);
		this->new_outer_state_zeroed.push_back(stoi(zeroed_line));
	}

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

StateNetwork::~StateNetwork() {
	delete this->obs_input;
	delete this->state_input;
	delete this->new_inner_state_input;
	delete this->new_outer_state_input;
	delete this->hidden;
	delete this->output;
}

void StateNetwork::activate(double obs_val,
							vector<double>& state_vals) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::activate(double obs_val,
							vector<double>& state_vals,
							StateNetworkHistory* history) {
	activate(obs_val,
			 state_vals);

	history->save_weights();
}

void StateNetwork::activate(vector<double>& state_vals) {
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::activate(vector<double>& state_vals,
							StateNetworkHistory* history) {
	activate(state_vals);

	history->save_weights();
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();

	for (int s_index = 0; s_index < this->state_size; s_index++) {
		state_errors[s_index] += this->state_input->errors[s_index];
		this->state_input->errors[s_index] = 0.0;
	}
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& state_errors,
		StateNetworkHistory* history) {
	history->reset_weights();

	backprop_errors_with_no_weight_change(output_error,
										  state_errors);
}

void StateNetwork::backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop_weights_with_no_error_signal();

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

void StateNetwork::backprop_weights_with_no_error_signal(
		double output_error,
		double target_max_update,
		StateNetworkHistory* history) {
	history->reset_weights();

	backprop_weights_with_no_error_signal(output_error,
										  target_max_update);
}

void StateNetwork::new_external_activate(double obs_val,
										 vector<double>& state_vals,
										 vector<double>& new_external_state_vals) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		this->new_outer_state_input->acti_vals[o_index] = new_external_state_vals[o_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::new_external_activate(double obs_val,
										 vector<double>& state_vals,
										 vector<double>& new_external_state_vals,
										 StateNetworkHistory* history) {
	new_external_activate(obs_val,
						  state_vals,
						  new_outer_state_vals);

	history->save_weights();
}

void StateNetwork::new_external_activate(vector<double>& state_vals,
										 vector<double>& new_external_state_vals) {
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		this->state_input->acti_vals[s_index] = state_vals[s_index];
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		this->new_outer_state_input->acti_vals[o_index] = new_external_state_vals[o_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::new_external_activate(vector<double>& state_vals,
										 vector<double>& new_external_state_vals,
										 StateNetworkHistory* history) {
	new_external_activate(state_vals,
						  new_external_state_vals);

	history->save_weights();
}

void StateNetwork::new_external_backprop(double output_error,
										 vector<double>& new_external_state_errors,
										 double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		new_external_state_errors[o_index] += this->new_outer_state_input->errors[o_index];
		this->new_outer_state_input->errors[o_index] = 0.0;
	}

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

void StateNetwork::new_external_backprop(double output_error,
										 vector<double>& new_external_state_errors,
										 double target_max_update,
										 StateNetworkHistory* history) {
	history->reset_weights();

	new_external_backprop(output_error,
						  new_external_state_errors,
						  target_max_update);
}

void StateNetwork::new_external_backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& new_external_state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();

	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		new_external_state_errors[o_index] += this->new_outer_state_input->errors[o_index];
		this->new_outer_state_input->errors[o_index] = 0.0;
	}
}

void StateNetwork::new_external_backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& new_external_state_errors,
		StateNetworkHistory* history) {
	history->reset_weights();

	new_external_backprop_errors_with_no_weight_change(
		output_error,
		new_external_state_errors);
}

void StateNetwork::new_sequence_activate(double obs_val,
										 vector<double>& new_inner_state_vals,
										 vector<double>& state_vals,
										 vector<double>& new_outer_state_vals) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		if (!this->new_inner_state_zeroed[i_index]) {
			this->new_inner_state_input->acti_vals[i_index] = new_inner_state_vals[i_index];
		}
	}
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		if (!this->state_zeroed[s_index]) {
			this->state_input->acti_vals[s_index] = state_vals[s_index];
		}
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		if (!this->new_outer_state_zeroed[o_index]) {
			this->new_outer_state_input->acti_vals[o_index] = new_outer_state_vals[o_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::new_sequence_activate(double obs_val,
										 vector<double>& new_inner_state_vals,
										 vector<double>& state_vals,
										 vector<double>& new_outer_state_vals,
										 StateNetworkHistory* history) {
	new_sequence_activate(obs_val,
						  new_inner_state_vals,
						  state_vals,
						  new_outer_state_vals);

	history->save_weights();
}

void StateNetwork::new_sequence_activate(vector<double>& new_inner_state_vals,
										 vector<double>& state_vals,
										 vector<double>& new_outer_state_vals) {
	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		if (!this->new_inner_state_zeroed[i_index]) {
			this->new_inner_state_input->acti_vals[i_index] = new_inner_state_vals[i_index];
		}
	}
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		if (!this->state_zeroed[s_index]) {
			this->state_input->acti_vals[s_index] = state_vals[s_index];
		}
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		if (!this->new_outer_state_zeroed[o_index]) {
			this->new_outer_state_input->acti_vals[o_index] = new_outer_state_vals[o_index];
		}
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::new_sequence_activate(vector<double>& new_inner_state_vals,
										 vector<double>& state_vals,
										 vector<double>& new_outer_state_vals,
										 StateNetworkHistory* history) {
	new_sequence_activate(new_inner_state_vals,
						  state_vals,
						  new_outer_state_vals);

	history->save_weights();
}

void StateNetwork::new_sequence_backprop(double output_error,
										 vector<double>& new_inner_state_errors,
										 vector<double>& state_errors,
										 vector<double>& new_outer_state_errors,
										 double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		if (!this->new_inner_state_zeroed[i_index]) {
			new_inner_state_errors[i_index] += this->new_inner_state_input->errors[i_index];
			this->new_inner_state_input->errors[i_index] = 0.0;
		}
	}
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		if (!this->state_zeroed[s_index]) {
			state_errors[s_index] += this->state_input->errors[s_index];
			this->state_input->errors[s_index] = 0.0;
		}
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		if (!this->new_outer_state_zeroed[o_index]) {
			new_outer_state_errors[o_index] += this->new_outer_state_input->errors[o_index];
			this->new_outer_state_input->errors[o_index] = 0.0;
		}
	}

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

void StateNetwork::new_sequence_backprop(double output_error,
										 vector<double>& new_inner_state_errors,
										 vector<double>& state_errors,
										 vector<double>& new_outer_state_errors,
										 double target_max_update,
										 StateNetworkHistory* history) {
	history->reset_weights();

	new_sequence_backprop(output_error,
						  new_inner_state_errors,
						  state_errors,
						  new_outer_state_errors,
						  target_max_update);
}

void StateNetwork::new_sequence_backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& new_inner_state_errors,
		vector<double>& state_errors,
		vector<double>& new_outer_state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop_errors_with_no_weight_change();
	this->hidden->backprop_errors_with_no_weight_change();

	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		if (!this->new_inner_state_zeroed[i_index]) {
			new_inner_state_errors[i_index] += this->new_inner_state_input->errors[i_index];
			this->new_inner_state_input->errors[i_index] = 0.0;
		}
	}
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		if (!this->state_zeroed[s_index]) {
			state_errors[s_index] += this->state_input->errors[s_index];
			this->state_input->errors[s_index] = 0.0;
		}
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		if (!this->new_outer_state_zeroed[o_index]) {
			new_outer_state_errors[o_index] += this->new_outer_state_input->errors[o_index];
			this->new_outer_state_input->errors[o_index] = 0.0;
		}
	}
}

void StateNetwork::new_sequence_backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& new_inner_state_errors,
		vector<double>& state_errors,
		vector<double>& new_outer_state_errors,
		StateNetworkHistory* history) {
	history->reset_weights();

	new_sequence_backprop_errors_with_no_weight_change(
		output_error,
		new_inner_state_errors,
		state_errors,
		new_outer_state_errors);
}

void StateNetwork::add_new_inner() {
	this->hidden->state_hidden_add_new_inner();

	this->new_inner_state_size++;
	this->new_inner_state_input->acti_vals.push_back(0.0);
	this->new_inner_state_input->errors.push_back(0.0);

	this->new_inner_state_zeroed.push_back(false);
}

void StateNetwork::add_new_outer() {
	this->hidden->state_hidden_add_new_outer();

	this->new_outer_state_size++;
	this->new_outer_state_input->acti_vals.push_back(0.0);
	this->new_outer_state_input->errors.push_back(0.0);

	this->new_outer_state_zeroed.push_back(false);
}

void StateNetwork::remove_new_outer() {
	this->hidden->state_hidden_remove_new_outer();

	this->new_outer_state_size = 0;
	this->new_outer_state_input->acti_vals.clear();
	this->new_outer_state_input->errors.clear();
	this->new_outer_state_zeroed.clear();
}

void StateNetwork::zero_state(int index) {
	if (index < this->new_inner_state_size) {
		this->new_inner_state_zeroed[index] = true;
		this->new_inner_state_input->acti_vals[index] = 0.0;
		this->new_inner_state_input->errors[index] = 0.0;
		this->hidden->state_hidden_zero_new_input(index);
	} else {
		index -= this->new_inner_state_size;
		if (index < this->state_size) {
			this->state_zeroed[index] = true;
			this->state_input->acti_vals[index] = 0.0;
			this->state_input->errors[index] = 0.0;
			this->hidden->state_hidden_zero_state(index);
		} else {
			index -= this->state_size;
			this->new_outer_state_zeroed[index] = true;
			this->new_outer_state_input->acti_vals[index] = 0.0;
			this->new_outer_state_input->errors[index] = 0.0;
			this->hidden->state_hidden_zero_new_outer(index);
		}
	}
}

void StateNetwork::update_state_size(int new_state_size) {
	int state_size_increase = new_state_size - this->state_size;
	for (int s_index = 0; s_index < state_size_increase; s_index++) {
		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);

		// not needed, but match for now
		this->state_zeroed.push_back(false);
	}
	this->state_size = new_state_size;

	this->hidden->state_hidden_update_state_size(state_size_increase);
}

void StateNetwork::new_external_to_state() {
	this->hidden->state_hidden_new_external_weights_to_state();

	this->state_size += this->new_outer_state_size;
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);

		this->state_zeroed.push_back(false);
	}

	this->new_outer_state_size = 0;
	this->new_outer_state_input->acti_vals.clear();
	this->new_outer_state_input->errors.clear();
	this->new_outer_state_zeroed.clear();

	// also clear new_inner in case empty step in sequence
	this->new_inner_state_size = 0;
	this->new_inner_state_input->acti_vals.clear();
	this->new_inner_state_input->errors.clear();
	this->new_inner_state_zeroed.clear();
}

void StateNetwork::new_sequence_finalize() {
	this->hidden->state_hidden_new_sequence_finalize();

	this->state_size += this->new_inner_state_size;
	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		this->state_input->acti_vals.insert(this->state_input->acti_vals.begin(), 0.0);
		this->state_input->errors.insert(this->state_input->errors.begin(), 0.0);

		this->state_zeroed.insert(this->state_zeroed.begin(), false);
	}
	this->new_inner_state_size = 0;
	this->new_inner_state_input->acti_vals.clear();
	this->new_inner_state_input->errors.clear();
	this->new_inner_state_zeroed.clear();

	this->state_size += this->new_outer_state_size;
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);

		this->state_zeroed.push_back(false);
	}
	this->new_outer_state_size = 0;
	this->new_outer_state_input->acti_vals.clear();
	this->new_outer_state_input->errors.clear();
	this->new_outer_state_zeroed.clear();
}

void StateNetwork::remove_state(int index) {
	this->hidden->state_hidden_remove_state(index);

	this->state_size--;
	this->state_input->acti_vals.pop_back();
	this->state_input->errors.pop_back();
	this->state_zeroed.pop_back();
}

void StateNetwork::add_state(int size) {
	this->hidden->state_hidden_add_state(size);

	this->state_size += size;
	for (int s_index = 0; s_index < size; s_index++) {
		this->state_input->acti_vals.push_back(0.0);
		this->state_input->errors.push_back(0.0);
		this->state_zeroed.push_back(false);
	}
}

void StateNetwork::save(ofstream& output_file) {
	output_file << this->obs_size << endl;
	output_file << this->state_size << endl;
	output_file << this->new_inner_state_size << endl;
	output_file << this->new_outer_state_size << endl;
	output_file << this->hidden_size << endl;
	
	for (int s_index = 0; s_index < this->state_size; s_index++) {
		output_file << this->state_zeroed[s_index] << endl;
	}
	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		output_file << this->new_inner_state_zeroed[i_index] << endl;
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		output_file << this->new_outer_state_zeroed[o_index] << endl;
	}

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);
}

StateNetworkHistory::StateNetworkHistory(StateNetwork* network) {
	this->network = network;
}

void StateNetworkHistory::save_weights() {
	if (this->network->obs_size > 0) {
		this->obs_input_history = this->network->obs_input->acti_vals[0];
	}
	this->state_input_history.reserve(this->network->state_size);
	for (int s_index = 0; s_index < this->network->state_size; s_index++) {
		this->state_input_history.push_back(this->network->state_input->acti_vals[s_index]);
	}
	this->new_inner_state_input_history.reserve(this->network->new_inner_state_size);
	for (int n_index = 0; n_index < this->network->new_inner_state_size; n_index++) {
		this->new_inner_state_input_history.push_back(this->network->new_inner_state_input->acti_vals[n_index]);
	}
	this->new_outer_state_input_history.reserve(this->network->new_outer_state_size);
	for (int n_index = 0; n_index < this->network->new_outer_state_size; n_index++) {
		this->new_outer_state_input_history.push_back(this->network->new_outer_state_input->acti_vals[n_index]);
	}
	this->hidden_history.reserve(this->network->hidden_size);
	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->hidden_history.push_back(this->network->hidden->acti_vals[n_index]);
	}
}

void StateNetworkHistory::reset_weights() {
	if (this->network->obs_size > 0) {
		this->network->obs_input->acti_vals[0] = this->obs_input_history;
	}
	// state_sizes may have increased, so use history size
	for (int s_index = 0; s_index < (int)this->state_input_history.size(); s_index++) {
		this->network->state_input->acti_vals[s_index] = this->state_input_history[s_index];
	}
	for (int n_index = 0; n_index < this->network->new_inner_state_size; n_index++) {
		this->network->new_inner_state_input->acti_vals[n_index] = this->new_inner_state_input_history[n_index];
	}
	for (int n_index = 0; n_index < this->network->new_outer_state_size; n_index++) {
		this->network->new_outer_state_input->acti_vals[n_index] = this->new_outer_state_input_history[n_index];
	}
	for (int n_index = 0; n_index < this->network->hidden_size; n_index++) {
		this->network->hidden->acti_vals[n_index] = this->hidden_history[n_index];
	}
}
