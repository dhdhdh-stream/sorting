#include "score_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

ScoreNetwork::ScoreNetwork(int num_states) {
	this->type = NETWORK_TYPE_SCORE;

	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(num_states);
	this->state_input->errors.resize(num_states);
	this->state_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(16);
	this->hidden_1->errors.resize(16);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(8);
	this->hidden_2->errors.resize(8);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(1);
	this->output->errors.resize(1);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->state_input);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(NETWORK_INIT_MULTIPLIER);
}

ScoreNetwork::ScoreNetwork(ScoreNetwork* original) {
	this->type = NETWORK_TYPE_SCORE;

	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(original->state_input->acti_vals.size());
	this->state_input->errors.resize(original->state_input->errors.size());
	this->state_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(original->hidden_1->acti_vals.size());
	this->hidden_1->errors.resize(original->hidden_1->errors.size());
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(original->hidden_2->acti_vals.size());
	this->hidden_2->errors.resize(original->hidden_2->errors.size());
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(original->output->acti_vals.size());
	this->output->errors.resize(original->output->errors.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->state_input);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(NETWORK_INIT_MULTIPLIER);
	this->output->copy_weights_from(original->output);
}

ScoreNetwork::ScoreNetwork(ifstream& input_file) {
	this->type = NETWORK_TYPE_SCORE;

	this->state_input = new Layer(LINEAR_LAYER);
	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);
	this->state_input->acti_vals.resize(num_states);
	this->state_input->errors.resize(num_states);
	this->state_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	this->hidden_1->acti_vals.resize(hidden_1_size);
	this->hidden_1->errors.resize(hidden_1_size);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->update_structure(NETWORK_INIT_MULTIPLIER);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	this->hidden_2->acti_vals.resize(hidden_2_size);
	this->hidden_2->errors.resize(hidden_2_size);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure(NETWORK_INIT_MULTIPLIER);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(1);
	this->output->errors.resize(1);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->state_input);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure(NETWORK_INIT_MULTIPLIER);

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

ScoreNetwork::~ScoreNetwork() {
	delete this->state_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void ScoreNetwork::activate(vector<double>& state_vals) {
	for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
		this->state_input->acti_vals(s_index) = state_vals[s_index];
	}

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->output->activate();
}

void ScoreNetwork::save(ScoreNetworkHistory* history) {
	history->state_input_history = vector<double>(this->state_input->acti_vals.size());
	for (int s_index = 0; s_index < (int)this->state_input->acti_vals.size(); s_index++) {
		history->state_input_history[s_index] = this->state_input->acti_vals(s_index);
	}
	history->hidden_1_history = vector<double>(this->hidden_1->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		history->hidden_1_history[h_index] = this->hidden_1->acti_vals(h_index);
	}
	history->hidden_2_history = vector<double>(this->hidden_2->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		history->hidden_2_history[h_index] = this->hidden_2->acti_vals(h_index);
	}
	history->output_history = this->output->acti_vals(0);
}

void ScoreNetwork::load(ScoreNetworkHistory* history) {
	for (int s_index = 0; s_index < (int)this->state_input->acti_vals.size(); s_index++) {
		this->state_input->acti_vals(s_index) = history->state_input_history[s_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		this->hidden_1->acti_vals(h_index) = history->hidden_1_history[h_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		this->hidden_2->acti_vals(h_index) = history->hidden_2_history[h_index];
	}
	this->output->acti_vals(0) = history->output_history;
}

void ScoreNetwork::init_backprop(double target_val) {
	this->output->errors(0) = target_val - this->output->acti_vals(0);

	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();
}

void ScoreNetwork::init_update(double& hidden_1_average_max_update,
							   double& hidden_2_average_max_update,
							   double& output_average_max_update) {
	double hidden_1_max_update = 0.0;
	this->hidden_1->get_max_update(hidden_1_max_update);
	hidden_1_average_max_update = 0.999*hidden_1_average_max_update+0.001*hidden_1_max_update;
	if (hidden_1_max_update > 0.0) {
		double hidden_1_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/hidden_1_average_max_update;
		if (hidden_1_learning_rate*hidden_1_max_update > NETWORK_TARGET_MAX_UPDATE) {
			hidden_1_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_1_max_update;
		}
		this->hidden_1->update_weights(hidden_1_learning_rate);
	}

	double hidden_2_max_update = 0.0;
	this->hidden_2->get_max_update(hidden_2_max_update);
	hidden_2_average_max_update = 0.999*hidden_2_average_max_update+0.001*hidden_2_max_update;
	if (hidden_2_max_update > 0.0) {
		double hidden_2_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/hidden_2_average_max_update;
		if (hidden_2_learning_rate*hidden_2_max_update > NETWORK_TARGET_MAX_UPDATE) {
			hidden_2_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_2_max_update;
		}
		this->hidden_2->update_weights(hidden_2_learning_rate);
	}

	double output_max_update = 0.0;
	this->output->get_max_update(output_max_update);
	output_average_max_update = 0.999*output_average_max_update+0.001*output_max_update;
	if (output_max_update > 0.0) {
		double output_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/output_average_max_update;
		if (output_learning_rate*output_max_update > NETWORK_TARGET_MAX_UPDATE) {
			output_learning_rate = NETWORK_TARGET_MAX_UPDATE/output_max_update;
		}
		this->output->update_weights(output_learning_rate);
	}
}

void ScoreNetwork::backprop(double target_val,
							vector<double>& state_errors) {
	this->output->errors(0) = target_val - this->output->acti_vals(0);

	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	for (int s_index = 0; s_index < (int)state_errors.size(); s_index++) {
		state_errors[s_index] += this->state_input->errors(s_index);
		this->state_input->errors(s_index) = 0.0;
	}
}

void ScoreNetwork::update_weights(double learning_rate) {
	this->hidden_1->update_weights(learning_rate);
	this->hidden_2->update_weights(learning_rate);
	this->output->update_weights(learning_rate);
}

void ScoreNetwork::add_states(int new_num_states) {
	this->state_input->acti_vals.resize(new_num_states);
	this->state_input->errors.resize(new_num_states);
	this->state_input->errors.setConstant(0.0);

	this->hidden_1->update_structure(0.0);
	this->hidden_2->update_structure(0.0);
	this->output->update_structure(0.0);
}

void ScoreNetwork::save(ofstream& output_file) {
	output_file << this->state_input->acti_vals.size() << endl;

	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->output->save_weights(output_file);
}

ScoreNetworkHistory::ScoreNetworkHistory(ScoreNetwork* network) {
	this->network = network;
}
