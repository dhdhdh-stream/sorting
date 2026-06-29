#include "network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 10;
/**
 * - not meaningful to update weights more often(?)
 */

Network::Network(int input_size) {
	this->raw_input = new Layer(LINEAR_LAYER);
	this->raw_input->acti_vals.resize(input_size);
	this->raw_input->errors.resize(input_size);
	this->raw_input->errors.setConstant(0.0);

	this->input_means.resize(input_size);
	this->input_means.setConstant(0.0);
	this->input_deviations.resize(input_size);
	this->input_deviations.setConstant(1.0);

	this->input = new Layer(LINEAR_LAYER);
	this->input->acti_vals.resize(input_size);
	this->input->errors.resize(input_size);
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(16);
	this->hidden_1->errors.resize(16);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_1_means.resize(16);
	this->hidden_1_means.setConstant(0.0);
	this->hidden_1_deviations.resize(16);
	this->hidden_1_deviations.setConstant(1.0);

	this->hidden_1_output = new Layer(LINEAR_LAYER);
	this->hidden_1_output->acti_vals.resize(16);
	this->hidden_1_output->errors.resize(16);
	this->hidden_1_output->errors.setConstant(0.0);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(8);
	this->hidden_2->errors.resize(8);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1_output);
	this->hidden_2->update_structure();

	this->hidden_2_means.resize(8);
	this->hidden_2_means.setConstant(0.0);
	this->hidden_2_deviations.resize(8);
	this->hidden_2_deviations.setConstant(1.0);

	this->hidden_2_output = new Layer(LINEAR_LAYER);
	this->hidden_2_output->acti_vals.resize(8);
	this->hidden_2_output->errors.resize(8);
	this->hidden_2_output->errors.setConstant(0.0);

	this->hidden_3 = new Layer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(4);
	this->hidden_3->errors.resize(4);
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1_output);
	this->hidden_3->input_layers.push_back(this->hidden_2_output);
	this->hidden_3->update_structure();

	this->hidden_3_means.resize(4);
	this->hidden_3_means.setConstant(0.0);
	this->hidden_3_deviations.resize(4);
	this->hidden_3_deviations.setConstant(1.0);

	this->hidden_3_output = new Layer(LINEAR_LAYER);
	this->hidden_3_output->acti_vals.resize(4);
	this->hidden_3_output->errors.resize(4);
	this->hidden_3_output->errors.setConstant(0.0);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(1);
	this->output->errors.resize(1);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1_output);
	this->output->input_layers.push_back(this->hidden_2_output);
	this->output->input_layers.push_back(this->hidden_3_output);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

Network::Network(Network* original) {
	this->raw_input = new Layer(LINEAR_LAYER);
	this->raw_input->acti_vals.resize(original->raw_input->acti_vals.size());
	this->raw_input->errors.resize(original->raw_input->errors.size());
	this->raw_input->errors.setConstant(0.0);

	this->input_means = original->input_means;
	this->input_deviations = original->input_deviations;

	this->input = new Layer(LINEAR_LAYER);
	this->input->acti_vals.resize(original->input->acti_vals.size());
	this->input->errors.resize(original->input->errors.size());
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(original->hidden_1->acti_vals.size());
	this->hidden_1->errors.resize(original->hidden_1->errors.size());
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_1_means = original->hidden_1_means;
	this->hidden_1_deviations = original->hidden_1_deviations;

	this->hidden_1_output = new Layer(LINEAR_LAYER);
	this->hidden_1_output->acti_vals.resize(original->hidden_1_output->acti_vals.size());
	this->hidden_1_output->errors.resize(original->hidden_1_output->errors.size());
	this->hidden_1_output->errors.setConstant(0.0);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(original->hidden_2->acti_vals.size());
	this->hidden_2->errors.resize(original->hidden_2->errors.size());
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1_output);
	this->hidden_2->update_structure();
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->hidden_2_means = original->hidden_2_means;
	this->hidden_2_deviations = original->hidden_2_deviations;

	this->hidden_2_output = new Layer(LINEAR_LAYER);
	this->hidden_2_output->acti_vals.resize(original->hidden_2_output->acti_vals.size());
	this->hidden_2_output->errors.resize(original->hidden_2_output->errors.size());
	this->hidden_2_output->errors.setConstant(0.0);

	this->hidden_3 = new Layer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(original->hidden_3->acti_vals.size());
	this->hidden_3->errors.resize(original->hidden_3->errors.size());
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1_output);
	this->hidden_3->input_layers.push_back(this->hidden_2_output);
	this->hidden_3->update_structure();
	this->hidden_3->copy_weights_from(original->hidden_3);

	this->hidden_3_means = original->hidden_3_means;
	this->hidden_3_deviations = original->hidden_3_deviations;

	this->hidden_3_output = new Layer(LINEAR_LAYER);
	this->hidden_3_output->acti_vals.resize(original->hidden_3_output->acti_vals.size());
	this->hidden_3_output->errors.resize(original->hidden_3_output->errors.size());
	this->hidden_3_output->errors.setConstant(0.0);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(original->output->acti_vals.size());
	this->output->errors.resize(original->output->errors.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1_output);
	this->output->input_layers.push_back(this->hidden_2_output);
	this->output->input_layers.push_back(this->hidden_3_output);
	this->output->update_structure();
	this->output->copy_weights_from(original->output);

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

Network::Network(ifstream& input_file) {
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);

	this->raw_input = new Layer(LINEAR_LAYER);
	this->raw_input->acti_vals.resize(input_size);
	this->raw_input->errors.resize(input_size);
	this->raw_input->errors.setConstant(0.0);

	this->input_means.resize(input_size);
	this->input_deviations.resize(input_size);
	for (int i_index = 0; i_index < input_size; i_index++) {
		string mean_line;
		getline(input_file, mean_line);
		this->input_means(i_index) = stod(mean_line);

		string deviation_line;
		getline(input_file, deviation_line);
		this->input_deviations(i_index) = stod(deviation_line);
	}

	this->input = new Layer(LINEAR_LAYER);
	this->input->acti_vals.resize(input_size);
	this->input->errors.resize(input_size);
	this->input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	this->hidden_1->acti_vals.resize(hidden_1_size);
	this->hidden_1->errors.resize(hidden_1_size);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_1_means.resize(hidden_1_size);
	this->hidden_1_deviations.resize(hidden_1_size);
	for (int i_index = 0; i_index < hidden_1_size; i_index++) {
		string mean_line;
		getline(input_file, mean_line);
		this->hidden_1_means(i_index) = stod(mean_line);

		string deviation_line;
		getline(input_file, deviation_line);
		this->hidden_1_deviations(i_index) = stod(deviation_line);
	}

	this->hidden_1_output = new Layer(LINEAR_LAYER);
	this->hidden_1_output->acti_vals.resize(hidden_1_size);
	this->hidden_1_output->errors.resize(hidden_1_size);
	this->hidden_1_output->errors.setConstant(0.0);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	this->hidden_2->acti_vals.resize(hidden_2_size);
	this->hidden_2->errors.resize(hidden_2_size);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1_output);
	this->hidden_2->update_structure();

	this->hidden_2_means.resize(hidden_2_size);
	this->hidden_2_deviations.resize(hidden_2_size);
	for (int i_index = 0; i_index < hidden_2_size; i_index++) {
		string mean_line;
		getline(input_file, mean_line);
		this->hidden_2_means(i_index) = stod(mean_line);

		string deviation_line;
		getline(input_file, deviation_line);
		this->hidden_2_deviations(i_index) = stod(deviation_line);
	}

	this->hidden_2_output = new Layer(LINEAR_LAYER);
	this->hidden_2_output->acti_vals.resize(hidden_2_size);
	this->hidden_2_output->errors.resize(hidden_2_size);
	this->hidden_2_output->errors.setConstant(0.0);

	this->hidden_3 = new Layer(LEAKY_LAYER);
	string hidden_3_size_line;
	getline(input_file, hidden_3_size_line);
	int hidden_3_size = stoi(hidden_3_size_line);
	this->hidden_3->acti_vals.resize(hidden_3_size);
	this->hidden_3->errors.resize(hidden_3_size);
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1_output);
	this->hidden_3->input_layers.push_back(this->hidden_2_output);
	this->hidden_3->update_structure();

	this->hidden_3_means.resize(hidden_3_size);
	this->hidden_3_deviations.resize(hidden_3_size);
	for (int i_index = 0; i_index < hidden_3_size; i_index++) {
		string mean_line;
		getline(input_file, mean_line);
		this->hidden_3_means(i_index) = stod(mean_line);

		string deviation_line;
		getline(input_file, deviation_line);
		this->hidden_3_deviations(i_index) = stod(deviation_line);
	}

	this->hidden_3_output = new Layer(LINEAR_LAYER);
	this->hidden_3_output->acti_vals.resize(hidden_3_size);
	this->hidden_3_output->errors.resize(hidden_3_size);
	this->hidden_3_output->errors.setConstant(0.0);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(1);
	this->output->errors.resize(1);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1_output);
	this->output->input_layers.push_back(this->hidden_2_output);
	this->output->input_layers.push_back(this->hidden_3_output);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->hidden_3->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->epoch_iter = 0;
	this->average_max_update = 0.0;
}

Network::~Network() {
	delete this->raw_input;
	delete this->input;
	delete this->hidden_1;
	delete this->hidden_1_output;
	delete this->hidden_2;
	delete this->hidden_2_output;
	delete this->hidden_3;
	delete this->hidden_3_output;
	delete this->output;
}

void Network::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->raw_input->acti_vals(i_index) = input_vals[i_index];
	}

	this->input->acti_vals = (this->raw_input->acti_vals - this->input_means).cwiseQuotient(this->input_deviations);

	this->hidden_1->activate();
	this->hidden_1_output->acti_vals = (this->hidden_1->acti_vals - this->hidden_1_means).cwiseQuotient(this->hidden_1_deviations);
	this->hidden_2->activate();
	this->hidden_2_output->acti_vals = (this->hidden_2->acti_vals - this->hidden_2_means).cwiseQuotient(this->hidden_2_deviations);
	this->hidden_3->activate();
	this->hidden_3_output->acti_vals = (this->hidden_3->acti_vals - this->hidden_3_means).cwiseQuotient(this->hidden_3_deviations);
	this->output->activate();
}

void Network::backprop(double error) {
	this->output->errors(0) = error;
	this->output->backprop();
	
	this->hidden_3->errors = this->hidden_3_output->errors.cwiseProduct(this->hidden_3_deviations);
	this->hidden_3_output->errors.setConstant(0.0);
	this->hidden_3_means = 0.99999*this->hidden_3_means + 0.00001*this->hidden_3->acti_vals;
	this->hidden_3_deviations = 0.99999*this->hidden_3_deviations
		+ 0.00001*(this->hidden_3->acti_vals - this->hidden_3_means).cwiseAbs();
	this->hidden_3->backprop();

	this->hidden_2->errors = this->hidden_2_output->errors.cwiseProduct(this->hidden_2_deviations);
	this->hidden_2_output->errors.setConstant(0.0);
	this->hidden_2_means = 0.99999*this->hidden_2_means + 0.00001*this->hidden_2->acti_vals;
	this->hidden_2_deviations = 0.99999*this->hidden_2_deviations
		+ 0.00001*(this->hidden_2->acti_vals - this->hidden_2_means).cwiseAbs();
	this->hidden_2->backprop();

	this->hidden_1->errors = this->hidden_1_output->errors.cwiseProduct(this->hidden_1_deviations);
	this->hidden_1_output->errors.setConstant(0.0);
	this->hidden_1_means = 0.99999*this->hidden_1_means + 0.00001*this->hidden_1->acti_vals;
	this->hidden_1_deviations = 0.99999*this->hidden_1_deviations
		+ 0.00001*(this->hidden_1->acti_vals - this->hidden_1_means).cwiseAbs();
	this->hidden_1->backprop();

	this->input_means = 0.99999*this->input_means + 0.00001*this->raw_input->acti_vals;
	this->input_deviations = 0.99999*this->input_deviations
		+ 0.00001*(this->raw_input->acti_vals - this->input_means).cwiseAbs();
}

void Network::update() {
	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
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

void Network::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;

	for (int i_index = 0; i_index < (int)this->input->acti_vals.size(); i_index++) {
		output_file << this->input_means[i_index] << endl;
		output_file << this->input_deviations[i_index] << endl;
	}

	output_file << this->hidden_1->acti_vals.size() << endl;

	for (int i_index = 0; i_index < (int)this->hidden_1->acti_vals.size(); i_index++) {
		output_file << this->hidden_1_means[i_index] << endl;
		output_file << this->hidden_1_deviations[i_index] << endl;
	}

	output_file << this->hidden_2->acti_vals.size() << endl;

	for (int i_index = 0; i_index < (int)this->hidden_2->acti_vals.size(); i_index++) {
		output_file << this->hidden_2_means[i_index] << endl;
		output_file << this->hidden_2_deviations[i_index] << endl;
	}

	output_file << this->hidden_3->acti_vals.size() << endl;

	for (int i_index = 0; i_index < (int)this->hidden_3->acti_vals.size(); i_index++) {
		output_file << this->hidden_3_means[i_index] << endl;
		output_file << this->hidden_3_deviations[i_index] << endl;
	}

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->hidden_3->save_weights(output_file);
	this->output->save_weights(output_file);
}
