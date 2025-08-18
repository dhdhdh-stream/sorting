#include "signal.h"

#include "signal_network.h"

using namespace std;

Signal::Signal() {
	this->match_network = NULL;
	this->score_network = NULL;
}

Signal::Signal(Signal* original) {
	this->match_input_is_pre = original->match_input_is_pre;
	this->match_input_indexes = original->match_input_indexes;
	this->match_input_obs_indexes = original->match_input_obs_indexes;
	this->match_network = new SignalNetwork(original->match_network);

	this->score_input_is_pre = original->score_input_is_pre;
	this->score_input_indexes = original->score_input_indexes;
	this->score_input_obs_indexes = original->score_input_obs_indexes;
	this->score_network = new SignalNetwork(original->score_network);
}

Signal::Signal(ifstream& input_file) {
	string match_input_size_line;
	getline(input_file, match_input_size_line);
	int match_input_size = stoi(match_input_size_line);
	for (int i_index = 0; i_index < match_input_size; i_index++) {
		string is_pre_line;
		getline(input_file, is_pre_line);
		this->match_input_is_pre.push_back(stoi(is_pre_line));

		string index_line;
		getline(input_file, index_line);
		this->match_input_indexes.push_back(stoi(index_line));

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->match_input_obs_indexes.push_back(stoi(obs_index_line));
	}
	this->match_network = new SignalNetwork(input_file);

	string score_input_size_line;
	getline(input_file, score_input_size_line);
	int score_input_size = stoi(score_input_size_line);
	for (int i_index = 0; i_index < score_input_size; i_index++) {
		string is_pre_line;
		getline(input_file, is_pre_line);
		this->score_input_is_pre.push_back(stoi(is_pre_line));

		string index_line;
		getline(input_file, index_line);
		this->score_input_indexes.push_back(stoi(index_line));

		string obs_index_line;
		getline(input_file, obs_index_line);
		this->score_input_obs_indexes.push_back(stoi(obs_index_line));
	}
	this->score_network = new SignalNetwork(input_file);
}

Signal::~Signal() {
	if (this->match_network != NULL) {
		delete this->match_network;
	}

	if (this->score_network != NULL) {
		delete this->score_network;
	}
}

void Signal::insert(bool is_pre,
					int index,
					int exit_index,
					int length) {
	for (int i_index = (int)this->match_input_is_pre.size()-1; i_index >= 0; i_index--) {
		if (this->match_input_is_pre[i_index] == is_pre) {
			if (this->match_input_indexes[i_index] >= exit_index) {
				this->match_input_indexes[i_index] += length - (exit_index - index);
			} else if (this->match_input_indexes[i_index] >= index) {
				this->match_input_is_pre.erase(this->match_input_is_pre.begin() + i_index);
				this->match_input_indexes.erase(this->match_input_indexes.begin() + i_index);
				this->match_input_obs_indexes.erase(this->match_input_obs_indexes.begin() + i_index);
				this->match_network->remove_input(i_index);
			}
		}
	}

	for (int i_index = (int)this->score_input_is_pre.size()-1; i_index >= 0; i_index--) {
		if (this->score_input_is_pre[i_index] == is_pre) {
			if (this->score_input_indexes[i_index] >= exit_index) {
				this->score_input_indexes[i_index] += length - (exit_index - index);
			} else if (this->score_input_indexes[i_index] >= index) {
				this->score_input_is_pre.erase(this->score_input_is_pre.begin() + i_index);
				this->score_input_indexes.erase(this->score_input_indexes.begin() + i_index);
				this->score_input_obs_indexes.erase(this->score_input_obs_indexes.begin() + i_index);
				this->score_network->remove_input(i_index);
			}
		}
	}
}

void Signal::save(ofstream& output_file) {
	output_file << this->match_input_is_pre.size() << endl;
	for (int i_index = 0; i_index < (int)this->match_input_is_pre.size(); i_index++) {
		output_file << this->match_input_is_pre[i_index] << endl;
		output_file << this->match_input_indexes[i_index] << endl;
		output_file << this->match_input_obs_indexes[i_index] << endl;
	}
	this->match_network->save(output_file);

	output_file << this->score_input_is_pre.size() << endl;
	for (int i_index = 0; i_index < (int)this->score_input_is_pre.size(); i_index++) {
		output_file << this->score_input_is_pre[i_index] << endl;
		output_file << this->score_input_indexes[i_index] << endl;
		output_file << this->score_input_obs_indexes[i_index] << endl;
	}
	this->score_network->save(output_file);
}
