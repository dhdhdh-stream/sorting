#include "eigen_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

EigenNetwork::EigenNetwork(int input_size,
						   int output_size) {
	this->input = new EigenLayer(LINEAR_LAYER);
	this->input->acti_vals.resize(input_size);
	this->input->errors.resize(input_size);
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new EigenLayer(LEAKY_LAYER);
	// this->hidden_1->acti_vals.resize(10);
	this->hidden_1->acti_vals.resize(100);
	// this->hidden_1->errors.resize(10);
	this->hidden_1->errors.resize(100);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new EigenLayer(LEAKY_LAYER);
	// this->hidden_2->acti_vals.resize(4);
	this->hidden_2->acti_vals.resize(40);
	// this->hidden_2->errors.resize(4);
	this->hidden_2->errors.resize(40);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new EigenLayer(LEAKY_LAYER);
	// this->hidden_3->acti_vals.resize(1);
	this->hidden_3->acti_vals.resize(10);
	// this->hidden_3->errors.resize(1);
	this->hidden_3->errors.resize(10);
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new EigenLayer(LINEAR_LAYER);
	this->output->acti_vals.resize(output_size);
	this->output->errors.resize(output_size);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

EigenNetwork::EigenNetwork(EigenNetwork* original) {
	this->input = new EigenLayer(LINEAR_LAYER);
	this->input->acti_vals.resize(original->input->acti_vals.size());
	this->input->errors.resize(original->input->errors.size());
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new EigenLayer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(original->hidden_1->acti_vals.size());
	this->hidden_1->errors.resize(original->hidden_1->errors.size());
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new EigenLayer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(original->hidden_2->acti_vals.size());
	this->hidden_2->errors.resize(original->hidden_2->errors.size());
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->hidden_3 = new EigenLayer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(original->hidden_3->acti_vals.size());
	this->hidden_3->errors.resize(original->hidden_3->errors.size());
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();
	this->hidden_3->copy_weights_from(original->hidden_3);

	this->output = new EigenLayer(LINEAR_LAYER);
	this->output->acti_vals.resize(original->output->acti_vals.size());
	this->output->errors.resize(original->output->errors.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();
	this->output->copy_weights_from(original->output);

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

EigenNetwork::EigenNetwork(ifstream& input_file) {
	this->input = new EigenLayer(LINEAR_LAYER);
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);
	this->input->acti_vals.resize(input_size);
	this->input->errors.resize(input_size);
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new EigenLayer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	this->hidden_1->acti_vals.resize(hidden_1_size);
	this->hidden_1->errors.resize(hidden_1_size);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new EigenLayer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	this->hidden_2->acti_vals.resize(hidden_2_size);
	this->hidden_2->errors.resize(hidden_2_size);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new EigenLayer(LEAKY_LAYER);
	string hidden_3_size_line;
	getline(input_file, hidden_3_size_line);
	int hidden_3_size = stoi(hidden_3_size_line);
	this->hidden_3->acti_vals.resize(hidden_3_size);
	this->hidden_3->errors.resize(hidden_3_size);
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new EigenLayer(LINEAR_LAYER);
	string output_size_line;
	getline(input_file, output_size_line);
	int output_size = stoi(output_size_line);
	this->output->acti_vals.resize(output_size);
	this->output->errors.resize(output_size);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->hidden_3->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

EigenNetwork::~EigenNetwork() {
	delete this->input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->hidden_3;
	delete this->output;
}

void EigenNetwork::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals(i_index) = input_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();
}

void EigenNetwork::backprop(vector<double>& errors) {
	for (int o_index = 0; o_index < (int)errors.size(); o_index++) {
		this->output->errors(o_index) = errors[o_index];
	}
	this->output->backprop();
	this->hidden_3->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		this->hidden_1->get_max_update(max_update);
		this->hidden_2->get_max_update(max_update);
		this->hidden_3->get_max_update(max_update);
		this->output->get_max_update(max_update);
		this->average_max_update = 0.999*this->average_max_update+0.001*max_update;
		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}
			this->hidden_1->update_weights(learning_rate);
			this->hidden_2->update_weights(learning_rate);
			this->hidden_3->update_weights(learning_rate);
			this->output->update_weights(learning_rate);
		}

		this->epoch_iter = 0;
	}
}

void EigenNetwork::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->hidden_3->acti_vals.size() << endl;
	output_file << this->output->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->hidden_3->save_weights(output_file);
	this->output->save_weights(output_file);
}
