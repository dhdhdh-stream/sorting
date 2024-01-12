#include "state_network.h"

#include <iostream>

#include "state.h"
#include "state_status.h"

using namespace std;

/**
 * - practical compromise
 *   - increasing decreases speed, but improves likelihood of not getting stuck
 */
const int HIDDEN_SIZE = 20;

const double TARGET_MAX_UPDATE = 0.05;

void StateNetwork::construct() {
	this->obs_input = new Layer(LINEAR_LAYER, 1);
	this->state_input = new Layer(LINEAR_LAYER, 1);

	this->hidden = new Layer(LEAKY_LAYER, HIDDEN_SIZE);
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

StateNetwork::StateNetwork(int index) {
	this->index = index;

	this->starting_mean = 0.0;
	this->starting_variance = 1.0;

	this->ending_mean = 0.0;
	this->ending_variance = 1.0;

	construct();
}

StateNetwork::StateNetwork(string path,
						   string name,
						   State* parent_state,
						   int index) {
	ifstream input_file;
	input_file.open(path + "saves/" + name + "/nns/" + to_string(parent_state->id) + "_" + to_string(index) + ".txt");

	this->parent_state = parent_state;
	this->index = index;

	string preceding_network_indexes_size_line;
	getline(input_file, preceding_network_indexes_size_line);
	int preceding_network_indexes_size = stoi(preceding_network_indexes_size_line);
	for (int n_index = 0; n_index < preceding_network_indexes_size; n_index++) {
		string network_index_line;
		getline(input_file, network_index_line);
		this->preceding_network_indexes.insert(stoi(network_index_line));
	}

	string starting_mean_line;
	getline(input_file, starting_mean_line);
	this->starting_mean = stod(starting_mean_line);

	string starting_standard_deviation_line;
	getline(input_file, starting_standard_deviation_line);
	this->starting_standard_deviation = stod(starting_standard_deviation_line);

	string ending_mean_line;
	getline(input_file, ending_mean_line);
	this->ending_mean = stod(ending_mean_line);

	string ending_standard_deviation_line;
	getline(input_file, ending_standard_deviation_line);
	this->ending_standard_deviation = stod(ending_standard_deviation_line);

	construct();

	this->hidden->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	input_file.close();
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
}

void StateNetwork::backprop(double& state_error) {
	this->output->errors[0] = state_error;

	this->output->backprop();
	this->hidden->backprop();

	state_error += this->state_input->errors[0];
	this->state_input->errors[0] = 0.0;

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

void StateNetwork::activate(double obs_val,
							StateStatus& state_status) {
	double starting_state_val;
	StateNetwork* last_network = state_status.last_network;
	if (last_network == NULL) {
		starting_state_val = (state_status.val * this->starting_standard_deviation) + this->starting_mean;
	} else if (this->parent_state != last_network->parent_state
			|| this->preceding_network_indexes.find(last_network->index) == this->preceding_network_indexes.end()) {
		double normalized = (state_status.val - last_network->ending_mean)
			/ last_network->ending_standard_deviation;
		starting_state_val = (normalized * this->starting_standard_deviation) + this->starting_mean;
	} else {
		starting_state_val = state_status.val;
	}

	this->obs_input->acti_vals[0] = obs_val;
	this->state_input->acti_vals[0] = starting_state_val;

	this->hidden->activate();
	this->output->activate();

	state_status.val += this->output->acti_vals[0];

	state_status.last_network = this;
}

void StateNetwork::save(string path,
						string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "/nns/" + to_string(this->parent_state->id) + "_" + to_string(this->index) + ".txt");

	output_file << this->preceding_network_indexes.size() << endl;
	for (set<int>::iterator it = preceding_network_indexes.begin();
			it != preceding_network_indexes.end(); it++) {
		output_file << *it << endl;
	}

	output_file << this->starting_mean << endl;
	output_file << this->starting_standard_deviation << endl;
	output_file << this->ending_mean << endl;
	output_file << this->ending_standard_deviation << endl;

	this->hidden->save_weights(output_file);
	this->output->save_weights(output_file);

	output_file.close();
}
