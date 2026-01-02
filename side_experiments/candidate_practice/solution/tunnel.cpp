#include "tunnel.h"

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

	this->try_history = original->try_history;
	this->val_history = original->val_history;
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

	string try_history_size_line;
	getline(input_file, try_history_size_line);
	int try_history_size = stoi(try_history_size_line);
	for (int h_index = 0; h_index < try_history_size; h_index++) {
		string val_line;
		getline(input_file, val_line);
		this->try_history.push_back(stoi(val_line));
	}

	string val_history_size_line;
	getline(input_file, val_history_size_line);
	int val_history_size = stoi(val_history_size_line);
	for (int h_index = 0; h_index < val_history_size; h_index++) {
		string val_line;
		getline(input_file, val_line);
		this->val_history.push_back(stod(val_line));
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

void Tunnel::update_vals(int num_runs) {
	double sum_vals = 0.0;
	for (int v_index = 0; v_index < (int)this->vals.size(); v_index++) {
		sum_vals += this->vals[v_index];
	}
	this->val_history.push_back(sum_vals / (double)num_runs);

	this->vals.clear();
}

const int LAST_NUM_CHECK = 5;
const double MIN_SUCCESS_PERCENT = 0.5;
bool Tunnel::is_valid() {
	if ((int)this->val_history.size() < LAST_NUM_CHECK) {
		return true;
	} else {
		int num_success = 0;
		for (int i_index = 0; i_index < LAST_NUM_CHECK; i_index++) {
			int index = (int)this->val_history.size() - 1 - i_index;
			if (this->val_history[index] == TUNNEL_TRY_STATUS_TRUE_SUCCESS) {
				num_success++;
			}
		}
		if (num_success >= MIN_SUCCESS_PERCENT * LAST_NUM_CHECK) {
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

	output_file << this->try_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->try_history.size(); h_index++) {
		output_file << this->try_history[h_index] << endl;
	}

	output_file << this->val_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->val_history.size(); h_index++) {
		output_file << this->val_history[h_index] << endl;
	}
}

void Tunnel::print() {
	cout << "this->obs_indexes:";
	for (int o_index = 0; o_index < (int)this->obs_indexes.size(); o_index++) {
		cout << " " << this->obs_indexes[o_index];
	}
	cout << endl;

	cout << "this->is_pattern: " << this->is_pattern << endl;

	cout << "this->try_history:";
	for (int h_index = 0; h_index < (int)this->try_history.size(); h_index++) {
		cout << " " << this->try_history[h_index];
	}
	cout << endl;

	cout << "this->val_history:";
	for (int h_index = 0; h_index < (int)this->val_history.size(); h_index++) {
		cout << " " << this->val_history[h_index];
	}
	cout << endl;
}
