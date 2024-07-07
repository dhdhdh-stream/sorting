#include "network.h"

#include <iostream>

#include "globals.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

const int INCREMENT_BASE_SIZE = 2;
const int NEW_LAYER_MULTIPLE = 4;

Network::Network() {
	this->input = new Layer(LINEAR_LAYER);

	this->hiddens.push_back(new Layer(LEAKY_LAYER));
	this->hiddens[0]->input_layers.push_back(this->input);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->input);
	this->output->input_layers.push_back(this->hiddens[0]);

	this->epoch_iter = 0;
	this->hidden_average_max_updates = vector<double>{0.0};
	this->output_average_max_update = 0.0;
}

Network::Network(Network* original) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < (int)original->input->acti_vals.size(); i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	for (int h_index = 0; h_index < (int)original->hiddens.size(); h_index++) {
		this->hiddens.push_back(new Layer(LEAKY_LAYER));
		for (int i_index = 0; i_index < (int)original->hiddens[h_index]->acti_vals.size(); i_index++) {
			this->hiddens[h_index]->acti_vals.push_back(0.0);
			this->hiddens[h_index]->errors.push_back(0.0);
		}

		this->hiddens[h_index]->input_layers.push_back(this->input);
		for (int hh_index = 0; hh_index < h_index; hh_index++) {
			this->hiddens[h_index]->input_layers.push_back(this->hiddens[hh_index]);
		}

		this->hiddens[h_index]->update_structure();
		this->hiddens[h_index]->copy_weights_from(original->hiddens[h_index]);
	}

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->input);
	for (int hh_index = 0; hh_index < (int)this->hiddens.size(); hh_index++) {
		this->output->input_layers.push_back(this->hiddens[hh_index]);
	}
	this->output->update_structure();
	this->output->copy_weights_from(original->output);

	this->epoch_iter = 0;
	this->hidden_average_max_updates = vector<double>(this->hiddens.size(), 0.0);
	this->output_average_max_update = 0.0;
}

Network::Network(ifstream& input_file) {
	this->input = new Layer(LINEAR_LAYER);
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	string num_hiddens_line;
	getline(input_file, num_hiddens_line);
	int num_hiddens = stoi(num_hiddens_line);
	for (int h_index = 0; h_index < num_hiddens; h_index++) {
		this->hiddens.push_back(new Layer(LEAKY_LAYER));

		string hidden_size_line;
		getline(input_file, hidden_size_line);
		int hidden_size = stoi(hidden_size_line);
		for (int i_index = 0; i_index < hidden_size; i_index++) {
			this->hiddens[h_index]->acti_vals.push_back(0.0);
			this->hiddens[h_index]->errors.push_back(0.0);
		}

		this->hiddens[h_index]->input_layers.push_back(this->input);
		for (int hh_index = 0; hh_index < h_index; hh_index++) {
			this->hiddens[h_index]->input_layers.push_back(this->hiddens[hh_index]);
		}

		this->hiddens[h_index]->update_structure();
	}

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->input);
	for (int hh_index = 0; hh_index < (int)this->hiddens.size(); hh_index++) {
		this->output->input_layers.push_back(this->hiddens[hh_index]);
	}
	this->output->update_structure();

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		this->hiddens[h_index]->load_weights_from(input_file);
	}
	this->output->load_weights_from(input_file);

	this->epoch_iter = 0;
	this->hidden_average_max_updates = vector<double>(this->hiddens.size(), 0.0);
	this->output_average_max_update = 0.0;
}

Network::~Network() {
	delete this->input;

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		delete this->hiddens[h_index];
	}

	delete this->output;
}

void Network::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = input_vals[i_index];
	}

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		this->hiddens[h_index]->activate();
	}

	this->output->activate();
}

void Network::backprop(double error) {
	this->output->errors[0] = error;
	this->output->backprop();

	for (int h_index = (int)this->hiddens.size()-1; h_index >= 0; h_index--) {
		this->hiddens[h_index]->backprop();
	}

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
			double hidden_max_update = 0.0;
			this->hiddens[h_index]->get_max_update(hidden_max_update);
			this->hidden_average_max_updates[h_index] = 0.999*this->hidden_average_max_updates[h_index]+0.001*hidden_max_update;
			if (hidden_max_update > 0.0) {
				double hidden_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_average_max_updates[h_index];
				if (hidden_learning_rate*hidden_max_update > NETWORK_TARGET_MAX_UPDATE) {
					hidden_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_max_update;
				}
				this->hiddens[h_index]->update_weights(hidden_learning_rate);
			}
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

void Network::increment(int new_num_inputs) {
	while ((int)this->input->acti_vals.size() < new_num_inputs) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	uniform_int_distribution<int> add_nodes_distribution(0, 4);
	if (add_nodes_distribution(generator) == 0) {
		for (int n_index = 0; n_index < INCREMENT_BASE_SIZE; n_index++) {
			this->hiddens[0]->acti_vals.push_back(0.0);
			this->hiddens[0]->errors.push_back(0.0);
		}
	}

	this->hiddens[0]->update_structure();

	for (int l_index = 1; l_index < (int)this->hiddens.size(); l_index++) {
		int curr_size = (int)this->hiddens[l_index]->acti_vals.size();
		if ((int)this->hiddens[l_index-1]->acti_vals.size() / NEW_LAYER_MULTIPLE > curr_size) {
			this->hiddens[l_index]->acti_vals.push_back(0.0);
			this->hiddens[l_index]->errors.push_back(0.0);
		}
		this->hiddens[l_index]->update_structure();
	}
	if ((int)this->hiddens.back()->acti_vals.size() / NEW_LAYER_MULTIPLE > 0) {
		this->hiddens.push_back(new Layer(LEAKY_LAYER));
		this->hiddens.back()->acti_vals.push_back(0.0);
		this->hiddens.back()->errors.push_back(0.0);
		this->hiddens.back()->input_layers.push_back(this->input);
		for (int l_index = 0; l_index < (int)this->hiddens.size()-1; l_index++) {
			this->hiddens.back()->input_layers.push_back(this->hiddens[l_index]);
		}
		this->hiddens.back()->update_structure();

		this->hidden_average_max_updates.push_back(0.0);

		this->output->input_layers.push_back(this->hiddens.back());
	}

	this->output->update_structure();
}

void Network::remove_input(int index) {
	this->input->acti_vals.erase(this->input->acti_vals.begin() + index);
	this->input->errors.erase(this->input->errors.begin() + index);

	for (int l_index = 0; l_index < (int)this->hiddens.size(); l_index++) {
		this->hiddens[l_index]->remove_input(index);
	}
	this->output->remove_input(index);
}

void Network::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;

	output_file << this->hiddens.size() << endl;
	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		output_file << this->hiddens[h_index]->acti_vals.size() << endl;
	}

	for (int h_index = 0; h_index < (int)this->hiddens.size(); h_index++) {
		this->hiddens[h_index]->save_weights(output_file);
	}
	this->output->save_weights(output_file);
}
