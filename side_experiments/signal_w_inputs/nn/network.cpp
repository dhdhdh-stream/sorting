#include "network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

Network::Network(int input_size) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 10; h_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 4; h_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 1; h_index++) {
		this->hidden_3->acti_vals.push_back(0.0);
		this->hidden_3->errors.push_back(0.0);
	}
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->hidden_3_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::Network(int input_size,
				 vector<double>& input_averages,
				 vector<double>& input_standard_deviations) {
	this->input_averages = input_averages;
	this->input_standard_deviations = input_standard_deviations;

	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 10; h_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 4; h_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 1; h_index++) {
		this->hidden_3->acti_vals.push_back(0.0);
		this->hidden_3->errors.push_back(0.0);
	}
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->hidden_3_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::Network(Network* original) {
	this->input_averages = original->input_averages;
	this->input_standard_deviations = original->input_standard_deviations;
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < (int)original->input->acti_vals.size(); i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int i_index = 0; i_index < (int)original->hidden_1->acti_vals.size(); i_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int i_index = 0; i_index < (int)original->hidden_2->acti_vals.size(); i_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->hidden_3 = new Layer(LEAKY_LAYER);
	for (int i_index = 0; i_index < (int)original->hidden_3->acti_vals.size(); i_index++) {
		this->hidden_3->acti_vals.push_back(0.0);
		this->hidden_3->errors.push_back(0.0);
	}
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();
	this->hidden_3->copy_weights_from(original->hidden_3);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();
	this->output->copy_weights_from(original->output);

	this->epoch_iter = 0;
	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->hidden_3_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::Network(ifstream& input_file) {
	string input_averages_size_line;
	getline(input_file, input_averages_size_line);
	int input_averages_size = stoi(input_averages_size_line);
	for (int i_index = 0; i_index < input_averages_size; i_index++) {
		string average_line;
		getline(input_file, average_line);
		this->input_averages.push_back(stod(average_line));
	}

	string input_standard_deviations_size_line;
	getline(input_file, input_standard_deviations_size_line);
	int input_standard_deviations_size = stoi(input_standard_deviations_size_line);
	for (int i_index = 0; i_index < input_standard_deviations_size; i_index++) {
		string standard_deviation_line;
		getline(input_file, standard_deviation_line);
		this->input_standard_deviations.push_back(stod(standard_deviation_line));
	}

	this->input = new Layer(LINEAR_LAYER);
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	for (int i_index = 0; i_index < hidden_1_size; i_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	for (int i_index = 0; i_index < hidden_2_size; i_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	string hidden_3_size_line;
	getline(input_file, hidden_3_size_line);
	int hidden_3_size = stoi(hidden_3_size_line);
	for (int i_index = 0; i_index < hidden_3_size; i_index++) {
		this->hidden_3->acti_vals.push_back(0.0);
		this->hidden_3->errors.push_back(0.0);
	}
	this->hidden_3->input_layers.push_back(this->input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->hidden_3->load_weights_from(input_file);
	this->output->load_weights_from(input_file);

	this->epoch_iter = 0;
	this->hidden_1_average_max_update = 0.0;
	this->hidden_2_average_max_update = 0.0;
	this->hidden_3_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

Network::~Network() {
	delete this->input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->hidden_3;
	delete this->output;
}

void Network::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = input_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();
}

void Network::activate(vector<double>& input_vals,
					   vector<bool>& input_is_on) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		if (input_is_on[i_index]) {
			double normalized = (input_vals[i_index] - this->input_averages[i_index])
				/ this->input_standard_deviations[i_index];
			if (normalized > INPUT_CLIP) {
				normalized = INPUT_CLIP;
			} else if (normalized < -INPUT_CLIP) {
				normalized = -INPUT_CLIP;
			}
			this->input->acti_vals[i_index] = normalized;
		} else {
			this->input->acti_vals[i_index] = 0.0;
		}
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();
}

void Network::backprop(double error) {
	this->output->errors[0] = error;
	this->output->backprop();
	this->hidden_3->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == NETWORK_EPOCH_SIZE) {
		double hidden_1_max_update = 0.0;
		this->hidden_1->get_max_update(hidden_1_max_update);
		this->hidden_1_average_max_update = 0.999*this->hidden_1_average_max_update+0.001*hidden_1_max_update;
		if (hidden_1_max_update > 0.0) {
			double hidden_1_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_1_average_max_update;
			if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_1_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_1_max_update;
			}
			this->hidden_1->update_weights(hidden_1_learning_rate);
		}

		double hidden_2_max_update = 0.0;
		this->hidden_2->get_max_update(hidden_2_max_update);
		this->hidden_2_average_max_update = 0.999*this->hidden_2_average_max_update+0.001*hidden_2_max_update;
		if (hidden_2_max_update > 0.0) {
			double hidden_2_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_2_average_max_update;
			if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_2_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_2_max_update;
			}
			this->hidden_2->update_weights(hidden_2_learning_rate);
		}

		double hidden_3_max_update = 0.0;
		this->hidden_3->get_max_update(hidden_3_max_update);
		this->hidden_3_average_max_update = 0.999*this->hidden_3_average_max_update+0.001*hidden_3_max_update;
		if (hidden_3_max_update > 0.0) {
			double hidden_3_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_3_average_max_update;
			if (hidden_3_learning_rate*hidden_3_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_3_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_3_max_update;
			}
			this->hidden_3->update_weights(hidden_3_learning_rate);
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

void Network::remove_input(int index) {
	this->input_averages.erase(this->input_averages.begin() + index);
	this->input_standard_deviations.erase(this->input_standard_deviations.begin() + index);
	this->input->acti_vals.erase(this->input->acti_vals.begin() + index);
	this->input->errors.erase(this->input->errors.begin() + index);

	this->hidden_1->remove_input(index);
	this->hidden_2->remove_input(index);
	this->hidden_3->remove_input(index);
}

void Network::save(ofstream& output_file) {
	output_file << this->input_averages.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_averages.size(); i_index++) {
		output_file << this->input_averages[i_index] << endl;
	}

	output_file << this->input_standard_deviations.size() << endl;
	for (int i_index = 0; i_index < (int)this->input_standard_deviations.size(); i_index++) {
		output_file << this->input_standard_deviations[i_index] << endl;
	}

	output_file << this->input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->hidden_3->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->hidden_3->save_weights(output_file);
	this->output->save_weights(output_file);
}
