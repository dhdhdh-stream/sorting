#include "signal.h"

#include "network.h"

using namespace std;

Signal::Signal() {
	this->match_network = NULL;
	this->score_network = NULL;
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
	this->match_network = new Network(input_file);

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
	this->score_network = new Network(input_file);
}

Signal::~Signal() {
	if (this->match_network != NULL) {
		delete this->match_network;
	}

	if (this->score_network != NULL) {
		delete this->score_network;
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
