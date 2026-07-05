#include "obs_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

ObsNetwork::ObsNetwork(int num_states,
					   int num_obs) {
	this->type = NETWORK_TYPE_OBS;

	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(num_states);
	this->state_input->errors.resize(num_states);
	this->state_input->errors.setConstant(0.0);

	this->raw_obs_input = new Layer(LINEAR_LAYER);
	this->raw_obs_input->acti_vals.resize(num_obs);
	this->raw_obs_input->errors.resize(num_obs);
	this->raw_obs_input->errors.setConstant(0.0);

	this->obs_input_means.resize(num_obs);
	this->obs_input_means.setConstant(0.0);
	this->obs_input_deviations.resize(num_obs);
	this->obs_input_deviations.setConstant(1.0);

	this->obs_input = new Layer(LINEAR_LAYER);
	this->obs_input->acti_vals.resize(num_obs);
	this->obs_input->errors.resize(num_obs);
	this->obs_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(HIDDEN_1_STATE_SIZE_MULTIPLE * num_states);
	this->hidden_1->errors.resize(HIDDEN_1_STATE_SIZE_MULTIPLE * num_states);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(HIDDEN_2_STATE_SIZE_MULTIPLE * num_states);
	this->hidden_2->errors.resize(HIDDEN_2_STATE_SIZE_MULTIPLE * num_states);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(HIDDEN_3_STATE_SIZE_MULTIPLE * num_states);
	this->hidden_3->errors.resize(HIDDEN_3_STATE_SIZE_MULTIPLE * num_states);
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->state_input);
	this->hidden_3->input_layers.push_back(this->obs_input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(num_states);
	this->output->errors.resize(num_states);
	this->output->errors.setConstant(0.0);
	/**
	 * - if directly connect input with no norm, can dominate hidden
	 */
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();
}

ObsNetwork::ObsNetwork(ObsNetwork* original) {
	this->type = NETWORK_TYPE_OBS;

	this->state_input = new Layer(LINEAR_LAYER);
	this->state_input->acti_vals.resize(original->state_input->acti_vals.size());
	this->state_input->errors.resize(original->state_input->errors.size());
	this->state_input->errors.setConstant(0.0);

	this->raw_obs_input = new Layer(LINEAR_LAYER);
	this->raw_obs_input->acti_vals.resize(original->raw_obs_input->acti_vals.size());
	this->raw_obs_input->errors.resize(original->raw_obs_input->errors.size());
	this->raw_obs_input->errors.setConstant(0.0);

	this->obs_input_means = original->obs_input_means;
	this->obs_input_deviations = original->obs_input_deviations;

	this->obs_input = new Layer(LINEAR_LAYER);
	this->obs_input->acti_vals.resize(original->obs_input->acti_vals.size());
	this->obs_input->errors.resize(original->obs_input->errors.size());
	this->obs_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	this->hidden_1->acti_vals.resize(original->hidden_1->acti_vals.size());
	this->hidden_1->errors.resize(original->hidden_1->errors.size());
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->update_structure();
	this->hidden_1->copy_weights_from(original->hidden_1);

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(original->hidden_2->acti_vals.size());
	this->hidden_2->errors.resize(original->hidden_2->errors.size());
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();
	this->hidden_2->copy_weights_from(original->hidden_2);

	this->hidden_3 = new Layer(LEAKY_LAYER);
	this->hidden_3->acti_vals.resize(original->hidden_3->acti_vals.size());
	this->hidden_3->errors.resize(original->hidden_3->errors.size());
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->state_input);
	this->hidden_3->input_layers.push_back(this->obs_input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();
	this->hidden_3->copy_weights_from(original->hidden_3);

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(original->output->acti_vals.size());
	this->output->errors.resize(original->output->errors.size());
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();
	this->output->copy_weights_from(original->output);
}

ObsNetwork::ObsNetwork(ifstream& input_file) {
	this->type = NETWORK_TYPE_OBS;

	this->state_input = new Layer(LINEAR_LAYER);
	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);
	this->state_input->acti_vals.resize(num_states);
	this->state_input->errors.resize(num_states);
	this->state_input->errors.setConstant(0.0);

	string num_obs_line;
	getline(input_file, num_obs_line);
	int num_obs = stoi(num_obs_line);

	this->raw_obs_input = new Layer(LINEAR_LAYER);
	this->raw_obs_input->acti_vals.resize(num_obs);
	this->raw_obs_input->errors.resize(num_obs);
	this->raw_obs_input->errors.setConstant(0.0);

	this->obs_input_means.resize(num_obs);
	this->obs_input_deviations.resize(num_obs);
	for (int i_index = 0; i_index < num_obs; i_index++) {
		string mean_line;
		getline(input_file, mean_line);
		this->obs_input_means(i_index) = stod(mean_line);

		string deviation_line;
		getline(input_file, deviation_line);
		this->obs_input_deviations(i_index) = stod(deviation_line);
	}

	this->obs_input = new Layer(LINEAR_LAYER);
	this->obs_input->acti_vals.resize(num_obs);
	this->obs_input->errors.resize(num_obs);
	this->obs_input->errors.setConstant(0.0);

	this->hidden_1 = new Layer(LEAKY_LAYER);
	string hidden_1_size_line;
	getline(input_file, hidden_1_size_line);
	int hidden_1_size = stoi(hidden_1_size_line);
	this->hidden_1->acti_vals.resize(hidden_1_size);
	this->hidden_1->errors.resize(hidden_1_size);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	string hidden_2_size_line;
	getline(input_file, hidden_2_size_line);
	int hidden_2_size = stoi(hidden_2_size_line);
	this->hidden_2->acti_vals.resize(hidden_2_size);
	this->hidden_2->errors.resize(hidden_2_size);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->hidden_3 = new Layer(LEAKY_LAYER);
	string hidden_3_size_line;
	getline(input_file, hidden_3_size_line);
	int hidden_3_size = stoi(hidden_3_size_line);
	this->hidden_3->acti_vals.resize(hidden_3_size);
	this->hidden_3->errors.resize(hidden_3_size);
	this->hidden_3->errors.setConstant(0.0);
	this->hidden_3->input_layers.push_back(this->state_input);
	this->hidden_3->input_layers.push_back(this->obs_input);
	this->hidden_3->input_layers.push_back(this->hidden_1);
	this->hidden_3->input_layers.push_back(this->hidden_2);
	this->hidden_3->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(1);
	this->output->errors.resize(1);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->input_layers.push_back(this->hidden_3);
	this->output->update_structure();

	this->hidden_1->load_weights_from(input_file);
	this->hidden_2->load_weights_from(input_file);
	this->hidden_3->load_weights_from(input_file);
	this->output->load_weights_from(input_file);
}

ObsNetwork::~ObsNetwork() {
	delete this->state_input;
	delete this->raw_obs_input;
	delete this->obs_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->hidden_3;
	delete this->output;
}

void ObsNetwork::activate(vector<double>& state_vals,
						  vector<double>& obs_input_vals) {
	for (int s_index = 0; s_index < (int)state_vals.size(); s_index++) {
		this->state_input->acti_vals(s_index) = state_vals[s_index];
	}

	for (int i_index = 0; i_index < (int)obs_input_vals.size(); i_index++) {
		this->raw_obs_input->acti_vals(i_index) = obs_input_vals[i_index];
	}
	this->obs_input->acti_vals = (this->raw_obs_input->acti_vals - this->obs_input_means).cwiseQuotient(this->obs_input_deviations);

	this->hidden_1->activate();
	this->hidden_2->activate();
	this->hidden_3->activate();
	this->output->activate();
}

void ObsNetwork::save(ObsNetworkHistory* history) {
	history->state_input_history = vector<double>(this->state_input->acti_vals.size());
	for (int s_index = 0; s_index < (int)this->state_input->acti_vals.size(); s_index++) {
		history->state_input_history[s_index] = this->state_input->acti_vals(s_index);
	}
	history->raw_obs_input_history = vector<double>(this->raw_obs_input->acti_vals.size());
	for (int i_index = 0; i_index < (int)this->raw_obs_input->acti_vals.size(); i_index++) {
		history->raw_obs_input_history[i_index] = this->raw_obs_input->acti_vals(i_index);
	}
	history->obs_input_history = vector<double>(this->obs_input->acti_vals.size());
	for (int i_index = 0; i_index < (int)this->obs_input->acti_vals.size(); i_index++) {
		history->obs_input_history[i_index] = this->obs_input->acti_vals(i_index);
	}
	history->hidden_1_history = vector<double>(this->hidden_1->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		history->hidden_1_history[h_index] = this->hidden_1->acti_vals(h_index);
	}
	history->hidden_2_history = vector<double>(this->hidden_2->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		history->hidden_2_history[h_index] = this->hidden_2->acti_vals(h_index);
	}
	history->hidden_3_history = vector<double>(this->hidden_3->acti_vals.size());
	for (int h_index = 0; h_index < (int)this->hidden_3->acti_vals.size(); h_index++) {
		history->hidden_3_history[h_index] = this->hidden_3->acti_vals(h_index);
	}
}

void ObsNetwork::load(ObsNetworkHistory* history) {
	for (int s_index = 0; s_index < (int)this->state_input->acti_vals.size(); s_index++) {
		this->state_input->acti_vals(s_index) = history->state_input_history[s_index];
	}
	for (int i_index = 0; i_index < (int)this->raw_obs_input->acti_vals.size(); i_index++) {
		this->raw_obs_input->acti_vals(i_index) = history->raw_obs_input_history[i_index];
	}
	for (int i_index = 0; i_index < (int)this->obs_input->acti_vals.size(); i_index++) {
		this->obs_input->acti_vals(i_index) = history->obs_input_history[i_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_1->acti_vals.size(); h_index++) {
		this->hidden_1->acti_vals(h_index) = history->hidden_1_history[h_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_2->acti_vals.size(); h_index++) {
		this->hidden_2->acti_vals(h_index) = history->hidden_2_history[h_index];
	}
	for (int h_index = 0; h_index < (int)this->hidden_3->acti_vals.size(); h_index++) {
		this->hidden_3->acti_vals(h_index) = history->hidden_3_history[h_index];
	}
}

void ObsNetwork::backprop(vector<double>& state_errors) {
	for (int s_index = 0; s_index < (int)state_errors.size(); s_index++) {
		this->output->errors(s_index) = state_errors[s_index];
	}
	this->output->backprop();
	this->hidden_3->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->obs_input_means = 0.99999*this->obs_input_means + 0.00001*this->raw_obs_input->acti_vals;
	this->obs_input_deviations = 0.99999*this->obs_input_deviations
		+ 0.00001*(this->raw_obs_input->acti_vals - this->obs_input_means).cwiseAbs();

	for (int s_index = 0; s_index < (int)state_errors.size(); s_index++) {
		state_errors[s_index] += this->state_input->acti_vals(s_index);
		this->state_input->acti_vals(s_index) = 0.0;
	}
}

void ObsNetwork::init_update(double& hidden_1_average_max_update,
							 double& hidden_2_average_max_update,
							 double& hidden_3_average_max_update,
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

	double hidden_3_max_update = 0.0;
	this->hidden_3->get_max_update(hidden_3_max_update);
	hidden_3_average_max_update = 0.999*hidden_3_average_max_update+0.001*hidden_3_max_update;
	if (hidden_3_max_update > 0.0) {
		double hidden_3_learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE)/hidden_3_average_max_update;
		if (hidden_3_learning_rate*hidden_3_max_update > NETWORK_TARGET_MAX_UPDATE) {
			hidden_3_learning_rate = NETWORK_TARGET_MAX_UPDATE/hidden_3_max_update;
		}
		this->hidden_3->update_weights(hidden_3_learning_rate);
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

void ObsNetwork::get_max_update(double& max_update) {
	this->hidden_1->get_max_update(max_update);
	this->hidden_2->get_max_update(max_update);
	this->hidden_3->get_max_update(max_update);
	this->output->get_max_update(max_update);
}

void ObsNetwork::update_weights(double learning_rate) {
	this->hidden_1->update_weights(learning_rate);
	this->hidden_2->update_weights(learning_rate);
	this->hidden_3->update_weights(learning_rate);
	this->output->update_weights(learning_rate);
}

void ObsNetwork::add_states(int new_num_states) {
	this->state_input->acti_vals.resize(new_num_states);
	this->state_input->errors.resize(new_num_states);
	this->state_input->errors.setConstant(0.0);

	this->hidden_1->acti_vals.resize(HIDDEN_1_STATE_SIZE_MULTIPLE * new_num_states);
	this->hidden_1->errors.resize(HIDDEN_1_STATE_SIZE_MULTIPLE * new_num_states);
	this->hidden_1->errors.setConstant(0.0);

	this->hidden_2->acti_vals.resize(HIDDEN_2_STATE_SIZE_MULTIPLE * new_num_states);
	this->hidden_2->errors.resize(HIDDEN_2_STATE_SIZE_MULTIPLE * new_num_states);
	this->hidden_2->errors.setConstant(0.0);

	this->hidden_3->acti_vals.resize(HIDDEN_3_STATE_SIZE_MULTIPLE * new_num_states);
	this->hidden_3->errors.resize(HIDDEN_3_STATE_SIZE_MULTIPLE * new_num_states);
	this->hidden_3->errors.setConstant(0.0);

	this->output->acti_vals.resize(new_num_states);
	this->output->errors.resize(new_num_states);
	this->output->errors.setConstant(0.0);

	this->hidden_1->update_structure();
	this->hidden_2->update_structure();
	this->hidden_3->update_structure();
	this->output->update_structure();
}

void ObsNetwork::save(ofstream& output_file) {
	output_file << this->state_input->acti_vals.size() << endl;

	output_file << this->obs_input->acti_vals.size() << endl;

	for (int i_index = 0; i_index < (int)this->obs_input->acti_vals.size(); i_index++) {
		output_file << this->obs_input_means[i_index] << endl;
		output_file << this->obs_input_deviations[i_index] << endl;
	}

	output_file << this->hidden_1->acti_vals.size() << endl;
	output_file << this->hidden_2->acti_vals.size() << endl;
	output_file << this->hidden_3->acti_vals.size() << endl;

	this->hidden_1->save_weights(output_file);
	this->hidden_2->save_weights(output_file);
	this->hidden_3->save_weights(output_file);
	this->output->save_weights(output_file);
}

ObsNetworkHistory::ObsNetworkHistory(ObsNetwork* network) {
	this->network = network;
}
