#include "long_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

const int NUM_HISTORIES = 1000;
const int INIT_NUM_CYCLES = 10;
const int ITERS_PER_CYCLE = 100;

LongNetwork::LongNetwork(int input_size) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 20; h_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 8; h_index++) {
		this->hidden_2->acti_vals.push_back(0.0);
		this->hidden_2->errors.push_back(0.0);
	}
	this->hidden_2->input_layers.push_back(this->input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	for (int h_index = 0; h_index < 2; h_index++) {
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

	this->history_index = 0;
}

LongNetwork::LongNetwork(LongNetwork* original) {
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

	this->history_index = 0;
}

LongNetwork::LongNetwork(ifstream& input_file) {
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

	this->history_index = 0;
}

LongNetwork::~LongNetwork() {
	delete this->input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->hidden_3;
	delete this->output;
}

void LongNetwork::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = input_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();
}

void LongNetwork::backprop(vector<double>& input_vals,
						   double error) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = input_vals[i_index];
	}
	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();

	double target_val = this->output->acti_vals[0] + error;

	if (this->input_histories.size() < NUM_HISTORIES) {
		this->input_histories.push_back(input_vals);
		this->target_val_histories.push_back(target_val);

		if (this->input_histories.size() >= NUM_HISTORIES) {
			for (int c_index = 0; c_index < INIT_NUM_CYCLES; c_index++) {
				backprop_cycle_helper();
			}
		}
	} else {
		this->input_histories[this->history_index] = input_vals;
		this->target_val_histories[this->history_index] = target_val;

		this->history_index++;

		if (this->history_index % ITERS_PER_CYCLE == 0) {
			backprop_cycle_helper();
		}

		if (this->history_index >= NUM_HISTORIES) {
			this->history_index = 0;
		}
	}
}

void LongNetwork::backprop_cycle_helper() {
	vector<int> remaining_indexes(NUM_HISTORIES);
	for (int i_index = 0; i_index < NUM_HISTORIES; i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	while (remaining_indexes.size() > 0) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);

		for (int i_index = 0; i_index < (int)this->input_histories[index].size(); i_index++) {
			this->input->acti_vals[i_index] = this->input_histories[index][i_index];
		}
		this->hidden_1->activate();
		this->hidden_2->activate();
		this->hidden_3->activate();
		this->output->activate();

		double error = this->target_val_histories[index] - this->output->acti_vals[0];

		this->output->errors[0] = error;
		this->output->backprop();
		this->hidden_3->backprop();
		this->hidden_2->backprop();
		this->hidden_1->backprop();

		remaining_indexes.erase(remaining_indexes.begin() + index);

		this->epoch_iter++;
		if (this->epoch_iter == EPOCH_SIZE) {
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
}

void LongNetwork::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;
	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->hidden_3->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->hidden_3->save_weights(output_file);
	this->output->save_weights(output_file);
}
