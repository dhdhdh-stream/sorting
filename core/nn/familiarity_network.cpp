#include "familiarity_network.h"

using namespace std;

const int HIDDEN_0_SIZE = 4;
const int HIDDEN_1_SIZE = 1;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 20;

FamiliarityNetwork::FamiliarityNetwork(int num_inputs) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < num_inputs; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_0 = new Layer(LEAKY_LAYER);
	for (int n_index = 0; n_index < HIDDEN_0_SIZE; n_index++) {
		this->hidden_0->acti_vals.push_back(0.0);
		this->hidden_0->errors.push_back(0.0);
	}
	this->hidden_0->input_layers.push_back(this->input);
	this->hidden_0->update_structure();

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int n_index = 0; n_index < HIDDEN_1_SIZE; n_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->input_layers.push_back(this->hidden_0);
	this->hidden_1->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden_0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->update_structure();

	this->epoch_iter = 0;
	this->hidden_0_average_max_update = 0.0;
	this->hidden_1_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FamiliarityNetwork::FamiliarityNetwork(FamiliarityNetwork* original) {
	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < (int)original->input->acti_vals.size(); i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_0 = new Layer(LEAKY_LAYER);
	for (int n_index = 0; n_index < HIDDEN_0_SIZE; n_index++) {
		this->hidden_0->acti_vals.push_back(0.0);
		this->hidden_0->errors.push_back(0.0);
	}
	this->hidden_0->input_layers.push_back(this->input);
	this->hidden_0->update_structure();
	this->hidden_0->copy_weights_from(original->hidden_0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int n_index = 0; n_index < HIDDEN_1_SIZE; n_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->input_layers.push_back(this->hidden_0);
	this->hidden_1->update_structure();
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden_0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->update_structure();
	this->output->copy_weights_from(original->output);

	this->epoch_iter = 0;
	this->hidden_0_average_max_update = 0.0;
	this->hidden_1_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FamiliarityNetwork::FamiliarityNetwork(ifstream& input_file) {
	string input_size_line;
	getline(input_file, input_size_line);
	int input_size = stoi(input_size_line);

	this->input = new Layer(LINEAR_LAYER);
	for (int i_index = 0; i_index < input_size; i_index++) {
		this->input->acti_vals.push_back(0.0);
		this->input->errors.push_back(0.0);
	}

	this->hidden_0 = new Layer(LEAKY_LAYER);
	for (int n_index = 0; n_index < HIDDEN_0_SIZE; n_index++) {
		this->hidden_0->acti_vals.push_back(0.0);
		this->hidden_0->errors.push_back(0.0);
	}
	this->hidden_0->input_layers.push_back(this->input);
	this->hidden_0->update_structure();
	this->hidden_0->load_weights_from(input_file);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	for (int n_index = 0; n_index < HIDDEN_1_SIZE; n_index++) {
		this->hidden_1->acti_vals.push_back(0.0);
		this->hidden_1->errors.push_back(0.0);
	}
	this->hidden_1->input_layers.push_back(this->input);
	this->hidden_1->input_layers.push_back(this->hidden_0);
	this->hidden_1->update_structure();
	this->hidden_1->load_weights_from(input_file);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.push_back(0.0);
	this->output->errors.push_back(0.0);
	this->output->input_layers.push_back(this->hidden_0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->update_structure();
	this->output->load_weights_from(input_file);

	this->epoch_iter = 0;
	this->hidden_0_average_max_update = 0.0;
	this->hidden_1_average_max_update = 0.0;
	this->output_average_max_update = 0.0;
}

FamiliarityNetwork::~FamiliarityNetwork() {
	delete this->input;
	delete this->hidden_0;
	delete this->hidden_1;
	delete this->output;
}

void FamiliarityNetwork::activate(vector<double>& input_vals) {
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[i_index] = input_vals[i_index];
	}
	this->hidden_0->activate();
	this->hidden_1->activate();
	this->output->activate();
}

void FamiliarityNetwork::backprop(double error) {
	this->output->errors[0] = error;
	this->output->backprop();
	this->hidden_1->backprop();
	this->hidden_0->backprop();

	this->epoch_iter++;
	if (this->epoch_iter == EPOCH_SIZE) {
		double hidden_0_max_update = 0.0;
		this->hidden_0->get_max_update(hidden_0_max_update);
		this->hidden_0_average_max_update = 0.999*this->hidden_0_average_max_update+0.001*hidden_0_max_update;
		if (hidden_0_max_update > 0.0) {
			double hidden_0_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/this->hidden_0_average_max_update;
			if (hidden_0_learning_rate*hidden_0_max_update > NETWORK_TARGET_MAX_UPDATE) {
				hidden_0_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_0_max_update;
			}
			this->hidden_0->update_weights(hidden_0_learning_rate);
		}

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

void FamiliarityNetwork::save(ofstream& output_file) {
	output_file << this->input->acti_vals.size() << endl;

	this->hidden_0->save_weights(output_file);
	this->hidden_1->save_weights(output_file);
	this->output->save_weights(output_file);
}
