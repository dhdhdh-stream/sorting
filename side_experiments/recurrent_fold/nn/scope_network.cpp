#include "scope_network.h"

#include <iostream>

using namespace std;

void ScopeNetwork::construct() {
	int input_state_size = this->original_input_state_size + this->new_input_state_size;
	this->input_state_input = new Layer(LINEAR_LAYER, input_state_size);
	int local_state_size = this->original_local_state_size + this->new_local_state_size;
	this->local_state_input = new Layer(LINEAR_LAYER, local_state_size);

	this->hidden = new Layer(LEAKY_LAYER, this->hidden_size);
	this->hidden->input_layers.push_back(this->input_state_input);
	this->hidden->input_layers.push_back(this->local_state_input);
	this->hidden->setup_weights_full();

	this->output = new Layer(LINEAR_LAYER, 1);
	this->output->input_layers.push_back(this->hidden);
	this->output->setup_weights_full();

	this->epoch_iter = 0;
	this->hidden_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

ScopeNetwork::ScopeNetwork(int original_input_state_size,
						   int original_local_state_size,
						   int new_input_state_size,
						   int new_local_state_size,
						   int hidden_size) {
	this->original_input_state_size = original_input_state_size;
	this->original_local_state_size = original_local_state_size;
	this->new_input_state_size = new_input_state_size;
	this->new_local_state_size = new_local_state_size;
	this->hidden_size = hidden_size;

	construct();
}

ScopeNetwork::ScopeNetwork(ScopeNetwork* original) {
	this->original_input_state_size = original->original_input_state_size;
	this->original_local_state_size = original->original_local_state_size;
	this->new_input_state_size = original->new_input_state_size;
	this->new_local_state_size = original->new_local_state_size;
	this->hidden_size = original->hidden_size;

	construct();

	this->hidden->copy_weights_from(original->hidden);
	this->output->copy_weights_from(original->output);
}

ScopeNetwork::ScopeNetwork(ifstream& input_file) {
	string original_input_state_size_line;
	getline(input_file, original_input_state_size_line);
	this->original_input_state_size = stoi(original_input_state_size_line);

	string original_local_state_size_line;
	getline(input_file, original_local_state_size_line);
	this->original_local_state_size = stoi(original_local_state_size_line);

	string new_input_state_size_line;
	getline(input_file, new_input_state_size_line);
	this->new_input_state_size = stoi(new_input_state_size_line);

	string new_local_state_size_line;
	getline(input_file, new_local_state_size_line);
	this->new_local_state_size = stoi(new_local_state_size_line);

	string hidden_size_line;
	getline(input_file, hidden_size_line);
	this->hidden_size = stoi(hidden_size_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

ScopeNetwork::~ScopeNetwork() {
	delete this->input_state_input;
	delete this->local_state_input;
	delete this->hidden;
	delete this->output;
}

void ScopeNetwork::activate(vector<double>& input_state_vals,
							vector<double>& local_state_vals) {
	int input_state_size = this->original_input_state_size + this->new_input_state_size;
	for (int i_index = 0; i_index < input_state_size; i_index++) {
		this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
	}
	int local_state_size = this->original_local_state_size + this->new_local_state_size;
	for (int l_index = 0; l_index < local_state_size; l_index++) {
		this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void ScopeNetwork::activate(vector<double>& input_state_vals,
							vector<double>& local_state_vals,
							ScopeNetworkHistory* history) {
	activate(input_state_vals,
			 local_state_vals);

	history->save_weights();
}

void ScopeNetwork::backprop(double output_error,
							vector<double>& input_state_errors,
							vector<double>& local_state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	int input_state_size = this->original_input_state_size + this->new_input_state_size;
	for (int i_index = 0; i_index < input_state_size; i_index++) {
		input_state_errors[i_index] += this->input_state_input->errors[i_index];
		this->input_state_input->errors[i_index] = 0.0;
	}
	int local_state_size = this->original_local_state_size + this->new_local_state_size;
	for (int l_index = 0; l_index < local_state_size; l_index++) {
		local_state_errors[l_index] += this->local_state_input->errors[l_index];
		this->local_state_input->errors[l_index] = 0.0;
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

void ScopeNetwork::backprop(double output_error,
							vector<double>& input_state_errors,
							vector<double>& local_state_errors,
							ScopeNetworkHistory* history) {
	history->reset_weights();

	backprop(output_error,
			 input_state_errors,
			 local_state_errors);
}

void ScopeNetwork::add_new_input_state() {
	this->new_input_state_size++;
	this->input_state_input->acti_vals.push_back(0.0);
	this->input_state_input->errors.push_back(0.0);

	this->hidden->scope_hidden_add_input_state();
}

void ScopeNetwork::new_input_activate(vector<double>& input_state_vals,
									  vector<double>& new_input_state_vals,
									  vector<double>& local_state_vals) {
	for (int i_index = 0; i_index < this->original_input_state_size; i_index++) {
		this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
	}
	for (int i_index = 0; i_index < this->new_input_state_size; i_index++) {
		this->input_state_input->acti_vals[this->original_input_state_size + i_index] = new_input_state_vals[i_index];
	}
	for (int l_index = 0; l_index < this->original_local_state_size; l_index++) {
		this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void ScopeNetwork::new_input_activate(vector<double>& input_state_vals,
									  vector<double>& new_input_state_vals,
									  vector<double>& local_state_vals,
									  ScopeNetworkHistory* history) {
	new_input_activate(input_state_vals,
					   new_input_state_vals,
					   local_state_vals);

	history->save_weights();
}

void ScopeNetwork::new_input_backprop(double output_error,
									  vector<double>& new_input_state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int i_index = 0; i_index < this->new_input_state_size; i_index++) {
		new_input_state_errors[i_index] += this->input_state_input->errors[this->original_input_state_size + i_index];
		this->input_state_input->errors[this->original_input_state_size + i_index] = 0.0;
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

void ScopeNetwork::new_input_backprop(double output_error,
									  vector<double>& new_input_state_errors,
									  ScopeNetworkHistory* history) {
	history->reset_weights();

	new_input_backprop(output_error,
					   new_input_state_errors);
}

void ScopeNetwork::add_new_local_state() {
	this->new_local_state_size++;
	this->local_state_input->acti_vals.push_back(0.0);
	this->local_state_input->errors.push_back(0.0);

	this->hidden->scope_hidden_add_local_state();
}

void ScopeNetwork::new_local_activate(vector<double>& input_state_vals,
									  vector<double>& local_state_vals,
									  vector<double>& new_local_state_vals) {
	for (int i_index = 0; i_index < this->original_input_state_size; i_index++) {
		this->input_state_input->acti_vals[i_index] = input_state_vals[i_index];
	}
	for (int l_index = 0; l_index < this->original_local_state_size; l_index++) {
		this->local_state_input->acti_vals[l_index] = local_state_vals[l_index];
	}
	for (int l_index = 0; l_index < this->new_local_state_size; l_index++) {
		this->local_state_input->acti_vals[this->original_local_state_size + l_index] = new_local_state_vals[l_index];
	}

	this->hidden->activate();
	this->output->activate();
}

void ScopeNetwork::new_local_activate(vector<double>& input_state_vals,
									  vector<double>& local_state_vals,
									  vector<double>& new_local_state_vals,
									  ScopeNetworkHistory* history) {
	new_local_activate(input_state_vals,
					   local_state_vals,
					   new_local_state_vals);

	history->save_weights();
}

void ScopeNetwork::new_local_backprop(double output_error,
									  vector<double>& new_local_state_errors) {
	this->output->errors[0] = output_error;

	this->output->backprop();
	this->hidden->backprop();

	for (int l_index = 0; l_index < this->new_local_state_size; l_index++) {
		new_local_state_errors[l_index] += this->local_state_input->errors[this->original_local_state_size + l_index];
		this->local_state_input->errors[this->original_local_state_size + l_index] = 0.0;
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

void ScopeNetwork::new_local_backprop(double output_error,
									  vector<double>& new_local_state_errors,
									  ScopeNetworkHistory* history) {
	history->reset_weights();

	new_local_backprop(output_error,
					   new_local_state_errors);
}

void ScopeNetwork::update_state_sizes(int input_state_size,
									  int local_state_size) {

}

void ScopeNetwork::add_input_state(int num_new_input_state) {

}

void ScopeNetwork::add_local_state(int num_new_local_state) {

}

void ScopeNetwork::save(ofstream& output_file) {

}

ScopeNetworkHistory::ScopeNetworkHistory(ScopeNetwork* network) {

}

void ScopeNetworkHistory::save_weights() {

}

void ScopeNetworkHistory::reset_weights() {

}
