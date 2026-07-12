#include "init_network.h"

#include <iostream>

#include "constants.h"
#include "globals.h"

using namespace std;

InitNetwork::InitNetwork(int num_states,
						 int num_obs) {
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
	this->hidden_1->acti_vals.resize(16);
	this->hidden_1->errors.resize(16);
	this->hidden_1->errors.setConstant(0.0);
	this->hidden_1->input_layers.push_back(this->state_input);
	this->hidden_1->input_layers.push_back(this->obs_input);
	this->hidden_1->update_structure();

	this->hidden_2 = new Layer(LEAKY_LAYER);
	this->hidden_2->acti_vals.resize(8);
	this->hidden_2->errors.resize(8);
	this->hidden_2->errors.setConstant(0.0);
	this->hidden_2->input_layers.push_back(this->state_input);
	this->hidden_2->input_layers.push_back(this->obs_input);
	this->hidden_2->input_layers.push_back(this->hidden_1);
	this->hidden_2->update_structure();

	this->output = new Layer(LINEAR_LAYER);
	this->output->acti_vals.resize(num_states);
	this->output->errors.resize(num_states);
	this->output->errors.setConstant(0.0);
	this->output->input_layers.push_back(this->hidden_1);
	this->output->input_layers.push_back(this->hidden_2);
	this->output->update_structure();
}

InitNetwork::~InitNetwork() {
	delete this->state_input;
	delete this->raw_obs_input;
	delete this->obs_input;
	delete this->hidden_1;
	delete this->hidden_2;
	delete this->output;
}

void InitNetwork::init_activate(vector<double>& state_vals,
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
	this->output->activate();

	for (int i_index = 0; i_index < (int)state_vals.size(); i_index++) {
		state_vals[i_index] += this->output->acti_vals(i_index);
	}
}

void InitNetwork::init_backprop(vector<double>& state_errors) {
	for (int i_index = 0; i_index < (int)state_errors.size(); i_index++) {
		this->output->errors(i_index) = state_errors[i_index];
	}
	this->output->backprop();
	this->hidden_2->backprop();
	this->hidden_1->backprop();

	this->obs_input_means = 0.99999*this->obs_input_means + 0.00001*this->raw_obs_input->acti_vals;
	this->obs_input_deviations = 0.99999*this->obs_input_deviations
		+ 0.00001*(this->raw_obs_input->acti_vals - this->obs_input_means).cwiseAbs();

	for (int s_index = 0; s_index < (int)state_errors.size(); s_index++) {
		state_errors[state_errors.size()-1 - s_index] += this->state_input->errors(this->state_input->errors.size()-1 - s_index);
		this->state_input->errors(this->state_input->errors.size()-1 - s_index) = 0.0;
	}
}

void InitNetwork::init_update(double& hidden_1_average_max_update,
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
