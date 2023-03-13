#include "state_network.h"

#include <iostream>

using namespace std;

void StateNetwork::construct() {
	this->obs_input = new Layer(LINEAR_LAYER, this->obs_size);
	this->local_state_input = new Layer(LINEAR_LAYER, this->local_state_size);
	this->input_state_input = new Layer(LINEAR_LAYER, this->input_state_size);
	this->new_inner_state_input = new Layer(LINEAR_LAYER, this->new_inner_state_size);
	this->new_outer_state_input = new Layer(LINEAR_LAYER, this->new_outer_state_size);

	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
	this->hidden->input_layers.push_back(this->obs_input);
	this->hidden->input_layers.push_back(this->local_state_input);
	this->hidden->input_layers.push_back(this->input_state_input);
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
						   int local_state_size,
						   int input_state_size,
						   int new_inner_state_size,
						   int new_outer_state_size,
						   int hidden_size) {
	this->obs_size = obs_size;
	this->local_state_size = local_state_size;
	this->input_state_size = input_state_size;
	this->new_inner_state_size = new_inner_state_size;
	this->new_outer_state_size = new_outer_state_size;
	this->hidden_size = hidden_size;

	construct();

	this->local_state_zeroed = vector<bool>(this->local_state_size, false);
	this->input_state_zeroed = vector<bool>(this->input_state_size, false);
	this->new_inner_state_zeroed = vector<bool>(this->new_inner_state_size, false);
	this->new_outer_state_zeroed = vector<bool>(this->new_outer_state_size, false);
}

StateNetwork::StateNetwork(StateNetwork* original) {
	this->obs_size = original->obs_size;
	this->local_state_size = original->local_state_size;
	this->input_state_size = original->input_state_size;
	this->new_inner_state_size = original->new_inner_state_size;
	this->new_outer_state_size = original->new_outer_state_size;
	this->hidden_size = original->hidden_size;

	construct();

	this->local_state_zeroed = original->local_state_zeroed;
	this->input_state_zeroed = original->input_state_zeroed;
	this->new_inner_state_zeroed = original->new_inner_state_zeroed;
	this->new_outer_state_zeroed = original->new_outer_state_zeroed;

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

StateNetwork::StateNetwork(ifstream& input_file) {
	string obs_size_line;
	getline(input_file, obs_size_line);
	this->obs_size = stoi(obs_size_line);

	string local_state_size_line;
	getline(input_file, local_state_size_line);
	this->local_state_size = stoi(local_state_size_line);

	string input_state_size_line;
	getline(input_file, input_state_size_line);
	this->input_state_size = stoi(input_state_size_line);

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

	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		string zeroed_line;
		getline(input_file, zeroed_line);
		this->local_state_zeroed.push_back(stoi(zeroed_line));
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		string zeroed_line;
		getline(input_file, zeroed_line);
		this->input_state_zeroed.push_back(stoi(zeroed_line));
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
	delete this->local_state_input;
	delete this->input_state_input;
	delete this->new_inner_state_input;
	delete this->new_outer_state_input;
	delete this->hidden;
	delete this->output;
}

void StateNetwork::activate(double obs_val,
							vector<double>& local_state_vals,
							vector<double>& input_state_vals) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::activate(double obs_val,
							vector<double>& local_state_vals,
							vector<double>& input_state_vals,
							StateNetworkHistory* history) {
	activate(obs_val,
			 local_state_vals,
			 input_state_vals);

	history->save_weights();
}

void StateNetwork::activate(vector<double>& local_state_vals,
							vector<double>& input_state_vals) {
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::activate(vector<double>& local_state_vals,
							vector<double>& input_state_vals,
							StateNetworkHistory* history) {
	activate(local_state_vals,
			 input_state_vals);

	history->save_weights();
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& local_state_errors,
		vector<double>& input_state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		local_state_errors[l_index] += this->local_state_input->errors[l_index];
		this->local_state_input->errors[l_index] = 0.0;
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		input_state_errors[i_index] += this->input_state_input->errors[i_index];
		this->input_state_input->errors[i_index] = 0.0;
	}
}

void StateNetwork::backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& local_state_errors,
		vector<double>& input_state_errors,
		StateNetworkHistory* history) {
	history->reset_weights();

	backprop_errors_with_no_weight_change(output_error,
										  local_state_errors,
										  input_state_errors);
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

void StateNetwork::new_outer_activate(double obs_val,
									  vector<double>& local_state_vals,
									  vector<double>& input_state_vals,
									  vector<double>& new_outer_state_vals) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		this->new_outer_state_input->acti_vals[o_index] = new_outer_state_vals[o_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::new_outer_activate(double obs_val,
									  vector<double>& local_state_vals,
									  vector<double>& input_state_vals,
									  vector<double>& new_outer_state_vals,
									  StateNetworkHistory* history) {
	new_outer_activate(obs_val,
					   local_state_vals,
					   input_state_vals,
					   new_outer_state_vals);

	history->save_weights();
}

void StateNetwork::new_outer_activate(vector<double>& local_state_vals,
									  vector<double>& input_state_vals,
									  vector<double>& new_outer_state_vals) {
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
	}
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		this->new_outer_state_input->acti_vals[o_index] = new_outer_state_vals[o_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void StateNetwork::new_outer_activate(vector<double>& local_state_vals,
									  vector<double>& input_state_vals,
									  vector<double>& new_outer_state_vals,
									  StateNetworkHistory* history) {
	new_outer_activate(local_state_vals,
					   input_state_vals,
					   new_outer_state_vals);

	history->save_weights();
}

void StateNetwork::new_outer_backprop(double output_error,
									  vector<double>& new_outer_state_errors,
									  double target_max_update) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->state_hidden_backprop_new_outer_state();

	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		new_outer_state_errors[o_index] += this->new_outer_state_input->errors[o_index];
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

void StateNetwork::new_outer_backprop(double output_error,
									  vector<double>& new_outer_state_errors,
									  double target_max_update,
									  StateNetworkHistory* history) {
	history->reset_weights();

	new_outer_backprop(output_error,
					   new_outer_state_errors,
					   target_max_update);
}

void StateNetwork::new_sequence_activate(double obs_val,
										 vector<double>& new_inner_state_vals,
										 vector<double>& local_state_vals,
										 vector<double>& input_state_vals,
										 vector<double>& new_outer_state_vals) {
	this->obs_input->acti_vals[0] = obs_val;
	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		if (!this->new_inner_state_zeroed[i_index]) {
			this->new_inner_state_input->acti_vals[i_index] = new_inner_state_vals[i_index];
		}
	}
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		if (!this->local_state_zeroed[l_index]) {
			this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
		}
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		if (!this->input_state_zeroed[i_index]) {
			this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
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
										 vector<double>& local_state_vals,
										 vector<double>& input_state_vals,
										 vector<double>& new_outer_state_vals,
										 StateNetworkHistory* history) {
	new_sequence_activate(obs_val,
						  new_inner_state_vals,
						  local_state_vals,
						  input_state_vals,
						  new_outer_state_vals);

	history->save_weights();
}

void StateNetwork::new_sequence_activate(vector<double>& new_inner_state_vals,
										 vector<double>& local_state_vals,
										 vector<double>& input_state_vals,
										 vector<double>& new_outer_state_vals) {
	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		if (!this->new_inner_state_zeroed[i_index]) {
			this->new_inner_state_input->acti_vals[i_index] = new_inner_state_vals[i_index];
		}
	}
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		if (!this->local_state_zeroed[l_index]) {
			this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
		}
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		if (!this->input_state_zeroed[i_index]) {
			this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
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
										 vector<double>& local_state_vals,
										 vector<double>& input_state_vals,
										 vector<double>& new_outer_state_vals,
										 StateNetworkHistory* history) {
	new_sequence_activate(new_inner_state_vals,
						  local_state_vals,
						  input_state_vals,
						  new_outer_state_vals);

	history->save_weights();
}

void StateNetwork::new_sequence_backprop(double output_error,
										 vector<double>& new_inner_state_errors,
										 vector<double>& local_state_errors,
										 vector<double>& input_state_errors,
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
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		if (!this->local_state_zeroed[l_index]) {
			local_state_errors[l_index] += this->local_state_input->errors[l_index];
			this->local_state_input->errors[l_index] = 0.0;
		}
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		if (!this->input_state_zeroed[i_index]) {
			input_state_errors[i_index] += this->input_state_input->errors[i_index];
			this->input_state_input->errors[i_index] = 0.0;
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
										 vector<double>& local_state_errors,
										 vector<double>& input_state_errors,
										 vector<double>& new_outer_state_errors,
										 double target_max_update,
										 StateNetworkHistory* history) {
	history->reset_weights();

	new_sequence_backprop(output_error,
						  new_inner_state_errors,
						  local_state_errors,
						  input_state_errors,
						  new_outer_state_errors,
						  target_max_update);
}

void StateNetwork::new_sequence_backprop_errors_with_no_weight_change(
		double output_error,
		vector<double>& new_inner_state_errors,
		vector<double>& local_state_errors,
		vector<double>& input_state_errors,
		vector<double>& new_outer_state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int i_index = 0; i_index < this->new_inner_state_size; i_index++) {
		if (!this->new_inner_state_zeroed[i_index]) {
			new_inner_state_errors[i_index] += this->new_inner_state_input->errors[i_index];
			this->new_inner_state_input->errors[i_index] = 0.0;
		}
	}
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		if (!this->local_state_zeroed[l_index]) {
			local_state_errors[l_index] += this->local_state_input->errors[l_index];
			this->local_state_input->errors[l_index] = 0.0;
		}
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		if (!this->input_state_zeroed[i_index]) {
			input_state_errors[i_index] += this->input_state_input->errors[i_index];
			this->input_state_input->errors[i_index] = 0.0;
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
		vector<double>& local_state_errors,
		vector<double>& input_state_errors,
		vector<double>& new_outer_state_errors,
		StateNetworkHistory* history) {
	history->reset_weights();

	new_sequence_backprop_errors_with_no_weight_change(
		output_error,
		new_inner_state_errors,
		local_state_errors,
		input_state_errors,
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
		if (index < this->local_state_size) {
			this->local_state_zeroed[index] = true;
			this->local_state_input->acti_vals[index] = 0.0;
			this->local_state_input->errors[index] = 0.0;
			this->hidden->state_hidden_zero_local(index);
		} else {
			index -= this->local_state_size;
			if (index < this->input_state_size) {
				this->input_state_zeroed[index] = true;
				this->input_state_input->acti_vals[index] = 0.0;
				this->input_state_input->errors[index] = 0.0;
				this->hidden->state_hidden_zero_input(index);
			} else {
				index -= this->input_state_size;
				this->new_outer_state_zeroed[index] = true;
				this->new_outer_state_input->acti_vals[index] = 0.0;
				this->new_outer_state_input->errors[index] = 0.0;
				this->hidden->state_hidden_zero_new_outer(index);
			}
		}
	}
}

void StateNetwork::update_state_sizes(int new_local_state_size,
									  int new_input_state_size) {
	int local_state_size_increase = new_local_state_size - this->local_state_size;
	for (int l_index = 0; l_index < local_state_size_increase; l_index++) {
		this->local_state_input->acti_vals.push_back(0.0);
		this->local_state_input->errors.push_back(0.0);

		// not needed, but match for now
		this->local_state_zeroed.push_back(false);
	}
	this->local_state_size = new_local_state_size;

	int input_state_size_increase = new_input_state_size - this->input_state_size;
	for (int i_index = 0; i_index < input_state_size_increase; i_index++) {
		this->input_state_input->acti_vals.push_back(0.0);
		this->input_state_input->errors.push_back(0.0);

		this->input_state_zeroed.push_back(false);
	}
	this->input_state_size = new_input_state_size;

	this->hidden->state_hidden_update_state_sizes(local_state_size_increase,
												  input_state_size_increase);
}

void StateNetwork::new_outer_to_local() {
	this->hidden->state_hidden_new_outer_weights_to_local();

	this->local_state_size += this->new_outer_state_size;
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		this->local_state_input->acti_vals.push_back(0.0);
		this->local_state_input->errors.push_back(0.0);

		this->local_state_zeroed.push_back(false);
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

void StateNetwork::new_outer_to_input() {
	this->hidden->state_hidden_new_outer_weights_to_input();

	this->input_state_size += this->new_outer_state_size;
	for (int o_index = 0; o_index < this->new_outer_state_size; o_index++) {
		this->input_state_input->acti_vals.push_back(0.0);
		this->input_state_input->errors.push_back(0.0);

		this->input_state_zeroed.push_back(false);
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

void StateNetwork::split_new_inner(int split_index) {
	this->hidden->state_hidden_split_new_inner(split_index);

	this->input_state_size = (this->new_inner_state_size - split_index)
		+ this->local_state_size
		+ this->input_state_size
		+ this->new_outer_state_size;
	this->input_state_input->acti_vals.clear();
	this->input_state_input->errors.clear();
	this->input_state_zeroed.clear();
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		this->input_state_input->acti_vals.push_back(0.0);
		this->input_state_input->errors.push_back(0.0);

		this->input_state_zeroed.push_back(false);
	}

	this->local_state_size = split_index;
	this->local_state_input->acti_vals.clear();
	this->local_state_input->errors.clear();
	this->local_state_zeroed.clear();
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		this->local_state_input->acti_vals.push_back(0.0);
		this->local_state_input->errors.push_back(0.0);

		this->local_state_zeroed.push_back(false);
	}

	this->new_inner_state_size = 0;
	this->new_inner_state_input->acti_vals.clear();
	this->new_inner_state_input->errors.clear();
	this->new_inner_state_zeroed.clear();

	this->new_outer_state_size = 0;
	this->new_outer_state_input->acti_vals.clear();
	this->new_outer_state_input->errors.clear();
	this->new_outer_state_zeroed.clear();
}

void StateNetwork::remove_local(int index) {
	this->hidden->state_hidden_remove_local(index);

	this->local_state_size--;
	this->local_state_input->acti_vals.pop_back();
	this->local_state_input->errors.pop_back();
	this->local_state_zeroed.pop_back();
}

void StateNetwork::remove_input(int index) {
	this->hidden->state_hidden_remove_input(index);

	this->input_state_size--;
	this->input_state_input->acti_vals.pop_back();
	this->input_state_input->errors.pop_back();
	this->input_state_zeroed.pop_back();
}

void StateNetwork::add_local(int size) {
	this->hidden->state_hidden_add_local(size);

	this->local_state_size += size;
	for (int s_index = 0; s_index < size; s_index++) {
		this->local_state_input->acti_vals.push_back(0.0);
		this->local_state_input->errors.push_back(0.0);
		this->local_state_zeroed.push_back(false);
	}
}

void StateNetwork::add_input(int size) {
	this->hidden->state_hidden_add_input(size);

	this->input_state_size += size;
	for (int s_index = 0; s_index < size; s_index++) {
		this->input_state_input->acti_vals.push_back(0.0);
		this->input_state_input->errors.push_back(0.0);
		this->input_state_zeroed.push_back(false);
	}
}

void StateNetwork::save(ofstream& output_file) {
	output_file << this->obs_size << endl;
	output_file << this->local_state_size << endl;
	output_file << this->input_state_size << endl;
	output_file << this->new_inner_state_size << endl;
	output_file << this->new_outer_state_size << endl;
	output_file << this->hidden_size << endl;
	
	for (int l_index = 0; l_index < this->local_state_size; l_index++) {
		output_file << this->local_state_zeroed[l_index] << endl;
	}
	for (int i_index = 0; i_index < this->input_state_size; i_index++) {
		output_file << this->input_state_zeroed[i_index] << endl;
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
	this->local_state_input_history.reserve(this->network->local_state_size);
	for (int n_index = 0; n_index < this->network->local_state_size; n_index++) {
		this->local_state_input_history.push_back(this->network->local_state_input->acti_vals[n_index]);
	}
	this->input_state_input_history.reserve(this->network->input_state_size);
	for (int n_index = 0; n_index < this->network->input_state_size; n_index++) {
		this->input_state_input_history.push_back(this->network->input_state_input->acti_vals[n_index]);
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
	for (int n_index = 0; n_index < (int)this->local_state_input_history.size(); n_index++) {
		this->network->local_state_input->acti_vals[n_index] = this->local_state_input_history[n_index];
	}
	for (int n_index = 0; n_index < (int)this->input_state_input_history.size(); n_index++) {
		this->network->input_state_input->acti_vals[n_index] = this->input_state_input_history[n_index];
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
