#include "network.h"

using namespace std;

void Network::add_input() {
	this->input_size++;
	this->input->acti_vals.push_back(0.0);
	this->input->errors.push_back(0.0);

	this->hidden->hidden_add_input();
}

void Network::remove_input(int index) {
	this->input_size--;
	this->input->acti_vals.pop_back();
	this->input->errors.pop_back();

	this->hidden->hidden_remove_input(index);
}

void Network::scope_activate(vector<double>& obs,
							 vector<double>& input_vals,
							 vector<double>& local_state_vals) {
	int input_index = 0;
	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		this->input->acti_vals[input_index] = obs[o_index];
		input_index++;
	}
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		if (input_index >= this->input_size) {
			// state networks only process first this->input_size states
			break;
		}
		this->input->acti_vals[input_index] = input_vals[i_index];
		input_index++;
	}
	for (int l_index = 0; l_index < (int)local_state_vals.size(); l_index++) {
		if (input_index >= this->input_size) {
			break;
		}
		this->input->acti_vals[input_index] = local_state_vals[l_index];
		input_index++;
	}

	this->hidden->activate();
	this->output->activate();
}

void Network::scope_activate(vector<double>& input_vals,
							 vector<double>& local_state_vals) {
	int input_index = 0;
	for (int i_index = 0; i_index < (int)input_vals.size(); i_index++) {
		this->input->acti_vals[input_index] = input_vals[i_index];
		input_index++;
	}
	for (int l_index = 0; l_index < (int)local_state_vals.size(); l_index++) {
		this->input->acti_vals[input_index] = local_state_vals[l_index];
		input_index++;
	}

	this->hidden->activate();
	this->output->activate();
}
