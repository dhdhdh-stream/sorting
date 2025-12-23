#include "tunnel.h"

#include <iostream>

#include "problem.h"
#include "solution_wrapper.h"

using namespace std;

Tunnel::Tunnel(bool is_default,
			   int obs_index) {
	this->is_default = is_default;
	this->obs_index = obs_index;

	this->num_tries = 0;
	this->num_successes = 0;
}

Tunnel::Tunnel(ifstream& input_file) {
	string is_default_line;
	getline(input_file, is_default_line);
	this->is_default = stoi(is_default_line);

	string obs_index_line;
	getline(input_file, obs_index_line);
	this->obs_index = stoi(obs_index_line);

	string num_tries_line;
	getline(input_file, num_tries_line);
	this->num_tries = stoi(num_tries_line);

	string num_successes_line;
	getline(input_file, num_successes_line);
	this->num_successes = stoi(num_successes_line);

	string num_improvements_line;
	getline(input_file, num_improvements_line);
	int num_improvements = stoi(num_improvements_line);
	for (int i_index = 0; i_index < num_improvements; i_index++) {
		string true_line;
		getline(input_file, true_line);
		this->true_improvements.push_back(stod(true_line));

		string signal_line;
		getline(input_file, signal_line);
		this->signal_improvements.push_back(stod(signal_line));
	}
}

double Tunnel::get_signal(SolutionWrapper* wrapper) {
	if (this->is_default) {
		double target_val = wrapper->problem->score_result();
		target_val -= 0.0001 * wrapper->num_actions;
		return target_val;
	} else {
		vector<double> obs = wrapper->problem->get_observations();
		return obs[this->obs_index];
	}
}

void Tunnel::save(ofstream& output_file) {
	output_file << this->is_default << endl;
	output_file << this->obs_index << endl;

	output_file << this->num_tries << endl;
	output_file << this->num_successes << endl;

	output_file << this->true_improvements.size() << endl;
	for (int i_index = 0; i_index < (int)this->true_improvements.size(); i_index++) {
		output_file << this->true_improvements[i_index] << endl;
		output_file << this->signal_improvements[i_index] << endl;
	}
}

void Tunnel::print() {
	cout << "this->is_default: " << this->is_default << endl;
	cout << "this->obs_index: " << this->obs_index << endl;
	cout << "this->num_tries: " << this->num_tries << endl;
	cout << "this->num_successes: " << this->num_successes << endl;
	for (int i_index = 0; i_index < (int)this->true_improvements.size(); i_index++) {
		cout << i_index << " " << this->true_improvements[i_index] << " " << this->signal_improvements[i_index] << endl;
	}
	cout << endl;
}
