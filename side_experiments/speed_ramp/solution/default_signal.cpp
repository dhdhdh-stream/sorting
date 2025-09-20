#include "default_signal.h"

#include "signal_network.h"

using namespace std;

DefaultSignal::DefaultSignal() {
	this->score_network = NULL;
}

DefaultSignal::DefaultSignal(DefaultSignal* original) {
	this->score_input_is_pre = original->score_input_is_pre;
	this->score_input_indexes = original->score_input_indexes;
	this->score_input_obs_indexes = original->score_input_obs_indexes;
	this->score_network = new SignalNetwork(original->score_network);
}

DefaultSignal::DefaultSignal(ifstream& input_file) {
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

DefaultSignal::~DefaultSignal() {
	if (this->score_network != NULL) {
		delete this->score_network;
	}
}

double DefaultSignal::calc(vector<vector<double>>& pre_obs_histories,
						   vector<vector<double>>& post_obs_histories) {
	vector<double> input_vals(this->score_input_is_pre.size());
	for (int i_index = 0; i_index < (int)this->score_input_is_pre.size(); i_index++) {
		if (this->score_input_is_pre[i_index]) {
			input_vals[i_index] = pre_obs_histories[
				this->score_input_indexes[i_index]][this->score_input_obs_indexes[i_index]];
		} else {
			input_vals[i_index] = post_obs_histories[
				this->score_input_indexes[i_index]][this->score_input_obs_indexes[i_index]];
		}
	}
	this->score_network->activate(input_vals);

	return this->score_network->output->acti_vals[0];
}

void DefaultSignal::save(ofstream& output_file) {
	output_file << this->score_input_is_pre.size() << endl;
	for (int i_index = 0; i_index < (int)this->score_input_is_pre.size(); i_index++) {
		output_file << this->score_input_is_pre[i_index] << endl;
		output_file << this->score_input_indexes[i_index] << endl;
		output_file << this->score_input_obs_indexes[i_index] << endl;
	}
	this->score_network->save(output_file);
}
