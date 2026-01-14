#include "tunnel.h"

#include <cmath>
#include <iostream>

#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

Tunnel::Tunnel(std::vector<int>& obs_indexes,
			   bool is_pattern,
			   Network* similarity_network,
			   Network* signal_network) {
	this->obs_indexes = obs_indexes;

	this->is_pattern = is_pattern;
	this->similarity_network = similarity_network;

	this->signal_network = signal_network;

	this->num_tries = 0;
	this->num_improve = 0;
}

Tunnel::Tunnel(Tunnel* original) {
	this->obs_indexes = original->obs_indexes;

	this->is_pattern = original->is_pattern;
	if (original->similarity_network == NULL) {
		this->similarity_network = NULL;
	} else {
		this->similarity_network = new Network(original->similarity_network);
	}

	this->signal_network = new Network(original->signal_network);

	this->num_tries = original->num_tries;
	this->num_improve = original->num_improve;

	this->starting_best_obs = original->starting_best_obs;
	this->starting_worst_obs = original->starting_worst_obs;
	this->starting_random_obs = original->starting_random_obs;

	this->latest_existing_obs = original->latest_existing_obs;
	this->latest_new_obs = original->latest_new_obs;
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

	string num_tries_line;
	getline(input_file, num_tries_line);
	this->num_tries = stoi(num_tries_line);

	string num_improve_line;
	getline(input_file, num_improve_line);
	this->num_improve = stoi(num_improve_line);

	for (int i_index = 0; i_index < 10; i_index++) {
		vector<double> obs;
		for (int o_index = 0; o_index < 25; o_index++) {
			string obs_line;
			getline(input_file, obs_line);
			obs.push_back(stod(obs_line));
		}
		this->starting_best_obs.push_back(obs);
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		vector<double> obs;
		for (int o_index = 0; o_index < 25; o_index++) {
			string obs_line;
			getline(input_file, obs_line);
			obs.push_back(stod(obs_line));
		}
		this->starting_worst_obs.push_back(obs);
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		vector<double> obs;
		for (int o_index = 0; o_index < 25; o_index++) {
			string obs_line;
			getline(input_file, obs_line);
			obs.push_back(stod(obs_line));
		}
		this->starting_random_obs.push_back(obs);
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		vector<double> obs;
		for (int o_index = 0; o_index < 25; o_index++) {
			string obs_line;
			getline(input_file, obs_line);
			obs.push_back(stod(obs_line));
		}
		this->latest_existing_obs.push_back(obs);
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		vector<double> obs;
		for (int o_index = 0; o_index < 25; o_index++) {
			string obs_line;
			getline(input_file, obs_line);
			obs.push_back(stod(obs_line));
		}
		this->latest_new_obs.push_back(obs);
	}
}

Tunnel::~Tunnel() {
	if (this->similarity_network != NULL) {
		delete this->similarity_network;
	}

	delete this->signal_network;
}

double Tunnel::get_signal(vector<double>& obs) {
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

bool Tunnel::is_fail() {
	if (this->num_tries < 10) {
		return false;
	} else {
		double improve_ratio = (double)this->num_improve / (double)this->num_tries;
		if (improve_ratio <= 0.2) {
			return true;
		} else {
			return false;
		}
	}
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

	output_file << this->num_tries << endl;
	output_file << this->num_improve << endl;

	for (int i_index = 0; i_index < 10; i_index++) {
		for (int o_index = 0; o_index < 25; o_index++) {
			output_file << this->starting_best_obs[i_index][o_index] << endl;
		}
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		for (int o_index = 0; o_index < 25; o_index++) {
			output_file << this->starting_worst_obs[i_index][o_index] << endl;
		}
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		for (int o_index = 0; o_index < 25; o_index++) {
			output_file << this->starting_random_obs[i_index][o_index] << endl;
		}
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		for (int o_index = 0; o_index < 25; o_index++) {
			output_file << this->latest_existing_obs[i_index][o_index] << endl;
		}
	}

	for (int i_index = 0; i_index < 10; i_index++) {
		for (int o_index = 0; o_index < 25; o_index++) {
			output_file << this->latest_new_obs[i_index][o_index] << endl;
		}
	}
}

void Tunnel::print() {
	cout << "this->obs_indexes:";
	for (int o_index = 0; o_index < (int)this->obs_indexes.size(); o_index++) {
		cout << " " << this->obs_indexes[o_index];
	}
	cout << endl;

	cout << "this->is_pattern: " << this->is_pattern << endl;

	cout << "this->num_tries: " << this->num_tries << endl;
	cout << "this->num_improve: " << this->num_improve << endl;
}

void Tunnel::print_obs() {
	cout << "starting_best_obs:" << endl;
	for (int i_index = 0; i_index < 10; i_index++) {
		cout << i_index << ":" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << this->starting_best_obs[i_index][5 * x_index + y_index] << " ";
			}
			cout << endl;
		}
	}

	cout << "starting_worst_obs:" << endl;
	for (int i_index = 0; i_index < 10; i_index++) {
		cout << i_index << ":" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << this->starting_best_obs[i_index][5 * x_index + y_index] << " ";
			}
			cout << endl;
		}
	}

	cout << "starting_random_obs:" << endl;
	for (int i_index = 0; i_index < 10; i_index++) {
		cout << i_index << ":" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << this->starting_best_obs[i_index][5 * x_index + y_index] << " ";
			}
			cout << endl;
		}
	}

	cout << "latest_existing_obs:" << endl;
	for (int i_index = 0; i_index < 10; i_index++) {
		cout << i_index << ":" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << this->starting_best_obs[i_index][5 * x_index + y_index] << " ";
			}
			cout << endl;
		}
	}

	cout << "latest_new_obs:" << endl;
	for (int i_index = 0; i_index < 10; i_index++) {
		cout << i_index << ":" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << this->starting_best_obs[i_index][5 * x_index + y_index] << " ";
			}
			cout << endl;
		}
	}
}
