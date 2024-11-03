#include "lstm.h"

#include <iostream>

#include "cell_network.h"
#include "gate.h"

using namespace std;

LSTM::LSTM(int num_obs,
		   int num_actions,
		   int num_states) {
	this->forget_gate = new Gate(num_obs,
								 num_actions,
								 num_states);
	this->input_gate = new Gate(num_obs,
								num_actions,
								num_states);
	this->output_gate = new Gate(num_obs,
								 num_actions,
								 num_states);
	this->cell_network = new CellNetwork(num_obs,
										 num_actions);
}

LSTM::LSTM(ifstream& input_file) {
	this->forget_gate = new Gate(input_file);
	this->input_gate = new Gate(input_file);
	this->output_gate = new Gate(input_file);
	this->cell_network = new CellNetwork(input_file);
}

LSTM::~LSTM() {
	delete this->forget_gate;
	delete this->input_gate;
	delete this->output_gate;
	delete this->cell_network;
}

void LSTM::activate(vector<double>& obs_vals,
					int action,
					vector<double>& state_vals) {
	this->forget_gate->activate(obs_vals,
								action,
								state_vals);
	this->input_gate->activate(obs_vals,
							   action,
							   state_vals);
	this->output_gate->activate(obs_vals,
								action,
								state_vals);
	this->cell_network->activate(obs_vals,
								 action);

	this->memory_val = this->forget_gate->output->acti_vals[0] * state_vals[this->index]
		+ this->input_gate->output->acti_vals[0] * this->cell_network->output->acti_vals[0];
	this->output = this->output_gate->output->acti_vals[0] * this->memory_val;
}

void LSTM::activate(vector<double>& obs_vals,
					int action,
					vector<double>& state_vals,
					LSTMHistory* history) {
	GateHistory* forget_gate_history = new GateHistory();
	this->forget_gate->activate(obs_vals,
								action,
								state_vals,
								forget_gate_history);
	history->forget_gate_history = forget_gate_history;

	GateHistory* input_gate_history = new GateHistory();
	this->input_gate->activate(obs_vals,
							   action,
							   state_vals,
							   input_gate_history);
	history->input_gate_history = input_gate_history;

	GateHistory* output_gate_history = new GateHistory();
	this->output_gate->activate(obs_vals,
								action,
								state_vals,
								output_gate_history);
	history->output_gate_history = output_gate_history;

	CellNetworkHistory* cell_network_history = new CellNetworkHistory();
	this->cell_network->activate(obs_vals,
								 action,
								 cell_network_history);
	history->cell_network_history = cell_network_history;

	history->prev_memory_val = state_vals[this->index];

	this->memory_val = this->forget_gate->output->acti_vals[0] * state_vals[this->index]
		+ this->input_gate->output->acti_vals[0] * this->cell_network->output->acti_vals[0];
	history->curr_memory_val = this->memory_val;

	this->output = this->output_gate->output->acti_vals[0] * this->memory_val;
}

void LSTM::backprop(double error,
					vector<double>& state_errors,
					LSTMHistory* history) {
	this->output_gate->backprop(error * history->curr_memory_val,
								state_errors,
								history->output_gate_history);

	double memory_error = error * history->output_gate_history->output_histories[0];

	state_errors[this->index] += memory_error * history->forget_gate_history->output_histories[0];

	this->forget_gate->backprop(memory_error * history->prev_memory_val,
								state_errors,
								history->forget_gate_history);

	this->input_gate->backprop(memory_error * history->cell_network_history->output_histories[0],
							   state_errors,
							   history->input_gate_history);

	this->cell_network->backprop(memory_error * history->input_gate_history->output_histories[0],
								 history->cell_network_history);
}

void LSTM::update_weights() {
	this->forget_gate->update_weights();
	this->input_gate->update_weights();
	this->output_gate->update_weights();
	this->cell_network->update_weights();
}

void LSTM::save(ofstream& output_file) {
	this->forget_gate->save(output_file);
	this->input_gate->save(output_file);
	this->output_gate->save(output_file);
	this->cell_network->save(output_file);
}

LSTMHistory::~LSTMHistory() {
	delete this->forget_gate_history;
	delete this->input_gate_history;
	delete this->output_gate_history;
	delete this->cell_network_history;
}
