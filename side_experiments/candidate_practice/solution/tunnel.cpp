#include "tunnel.h"

#include <iostream>

#include "network.h"
#include "problem.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

Tunnel::Tunnel(std::vector<int>& obs_indexes,
			   bool is_pattern,
			   Network* similarity_network,
			   Network* signal_network,
			   SolutionWrapper* wrapper) {
	this->obs_indexes = obs_indexes;

	this->is_pattern = is_pattern;
	this->similarity_network = similarity_network;

	this->signal_network = signal_network;

	this->starting_true = wrapper->solution->curr_score;
	this->ending_true = wrapper->solution->curr_score;

	this->num_tries = 0;
	this->num_train_fail = 0;
	this->num_measure_fail = 0;
	this->num_success = 0;
}

Tunnel::Tunnel(ifstream& input_file) {
	string num_obs_indexes_line;
	getline(input_file, num_obs_indexes_line);
	int num_obs_indexes = stoi(num_obs_indexes_line);
	for (int o_index = 0; o_index < num_obs_indexes; o_index++) {
		string index_line;
		getline(input_file, index_line);
		this->obs_indexes.push_back(stoi(index_line));
	}

	string is_pattern_line;
	getline(input_file, is_pattern_line);
	this->is_pattern = stoi(is_pattern_line);
	if (this->is_pattern) {
		this->similarity_network = new Network(input_file);
	} else {
		this->similarity_network = NULL;
	}

	this->signal_network = new Network(input_file);

	string starting_true_line;
	getline(input_file, starting_true_line);
	this->starting_true = stod(starting_true_line);

	string ending_true_line;
	getline(input_file, ending_true_line);
	this->ending_true = stod(ending_true_line);

	string num_tries_line;
	getline(input_file, num_tries_line);
	this->num_tries = stoi(num_tries_line);

	string num_train_fail_line;
	getline(input_file, num_train_fail_line);
	this->num_train_fail = stoi(num_train_fail_line);

	string num_measure_fail_line;
	getline(input_file, num_measure_fail_line);
	this->num_measure_fail = stoi(num_measure_fail_line);

	string num_success_line;
	getline(input_file, num_success_line);
	this->num_success = stoi(num_success_line);
}

Tunnel::~Tunnel() {
	if (this->similarity_network != NULL) {
		delete this->similarity_network;
	}

	delete this->signal_network;
}

double Tunnel::get_signal(SolutionWrapper* wrapper) {
	vector<double> obs = wrapper->problem->get_observations();

	vector<double> inputs(this->obs_indexes.size());
	for (int o_index = 0; o_index < (int)this->obs_indexes.size(); o_index++) {
		inputs[o_index] = obs[this->obs_indexes[o_index]];
	}

	double similarity;
	if (this->is_pattern) {
		this->similarity_network->activate(inputs);
		similarity = this->similarity_network->output->acti_vals[0];
		if (similarity > 1.0) {
			similarity = 1.0;
		} else if (similarity < 0.0) {
			similarity = 0.0;
		}
	} else {
		similarity = 1.0;
	}

	this->signal_network->activate(inputs);
	double signal = this->signal_network->output->acti_vals[0];

	return similarity * signal;
}

void Tunnel::save(ofstream& output_file) {
	output_file << this->obs_indexes.size() << endl;
	for (int o_index = 0; o_index < (int)this->obs_indexes.size(); o_index++) {
		output_file << this->obs_indexes[o_index] << endl;
	}

	output_file << this->is_pattern << endl;
	if (this->is_pattern) {
		this->similarity_network->save(output_file);
	}

	this->signal_network->save(output_file);

	output_file << this->starting_true << endl;
	output_file << this->ending_true << endl;

	output_file << this->num_tries << endl;
	output_file << this->num_train_fail << endl;
	output_file << this->num_measure_fail << endl;
	output_file << this->num_success << endl;
}

void Tunnel::print() {
	cout << "this->obs_indexes:";
	for (int o_index = 0; o_index < (int)this->obs_indexes.size(); o_index++) {
		cout << " " << this->obs_indexes[o_index];
	}
	cout << endl;
	cout << "this->starting_true: " << this->starting_true << endl;
	cout << "this->ending_true: " << this->ending_true << endl;
	cout << "this->num_tries: " << this->num_tries << endl;
	cout << "this->num_train_fail: " << this->num_train_fail << endl;
	cout << "this->num_measure_fail: " << this->num_measure_fail << endl;
	cout << "this->num_success: " << this->num_success << endl;
}
