#include "fold.h"

using namespace std;

Fold::Fold(int sequence_length) {
	this->sequence_length = sequence_length;

	this->outer_network = new Network(0, 20, 1);

	this->num_states = 0;
	this->states_updated = vector<int>(this->sequence_length, 0);
	this->state_networks = vector<vector<Network*>>(this->sequence_length, vector<Network*>());

	add_state();
}

Fold::~Fold() {
	delete this->outer_network;

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		for (int s_index = 0; s_index < (int)this->state_networks[f_index].size(); s_index++) {
			delete this->state_networks[f_index][s_index];
		}
	}
}

void Fold::activate(vector<vector<double>>& flat_vals) {
	vector<double> state_vals(this->num_states, 0.0);

	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		int starting_state = this->states_updated[f_index];
		for (int s_index = 0; s_index < (int)state_networks[f_index].size(); s_index++) {
			int target_state = starting_state+s_index;
			vector<double> state_network_input = flat_vals[f_index];
			for (int i_index = 0; i_index < target_state+1; i_index++) {
				state_network_input.push_back(state_vals[i_index]);
			}
			this->state_networks[f_index][s_index]->activate(state_network_input);
			state_vals[target_state] += this->state_networks[f_index][s_index]->output->acti_vals[0];
		}
	}

	// TODO: if states_updated[i+1] < states_updated[i], clear state_vals

	this->outer_network->activate(state_vals);
}

void Fold::backprop(double target_val) {
	double outer_error = target_val - this->outer_network->output->acti_vals[0];
	vector<double> outer_errors{outer_error};
	this->outer_network->backprop(outer_errors, 0.01);

	vector<double> state_errors(this->num_states);
	for (int s_index = 0; s_index < this->num_states; s_index++) {
		state_errors[s_index] = this->outer_network->input->errors[s_index];
		this->outer_network->input->errors[s_index] = 0.0;
	}

	for (int f_index = this->sequence_length-1; f_index >= 0; f_index--) {
		int starting_state = this->states_updated[f_index];
		for (int s_index = (int)state_networks[f_index].size()-1; s_index >= 0; s_index--) {
			int target_state = starting_state+s_index;
			vector<double> state_network_output_errors{state_errors[target_state]};
			this->state_networks[f_index][s_index]->backprop(state_network_output_errors, 0.01);
			for (int i_index = 0; i_index < target_state+1; i_index++) {
				state_errors[i_index] += this->state_networks[f_index][s_index]->input->errors[i_index];
				this->state_networks[f_index][s_index]->input->errors[i_index] = 0.0;
			}
		}
	}
}

void Fold::add_state() {
	this->outer_network->add_input();

	this->num_states++;
	
	for (int f_index = 0; f_index < this->sequence_length; f_index++) {
		int new_input_size = 1 + this->num_states;	// flat size always 1 for now
		this->state_networks[f_index].push_back(new Network(new_input_size,
															20,
															1));
	}
}
