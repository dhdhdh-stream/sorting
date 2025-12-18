#include "signal_experiment.h"

using namespace std;

SignalExperiment::SignalExperiment() {
	// do nothing
}

SignalExperiment::SignalExperiment(ifstream& input_file) {
	this->network = new Network(input_file);

	string signal_history_size_line;
	getline(input_file, signal_history_size_line);
	int signal_history_size = stoi(signal_history_size_line);
	for (int h_index = 0; h_index < signal_history_size; h_index++) {
		string val_line;
		getline(input_file, val_line);
		this->signal_history.push_back(stod(val_line));
	}
}

SignalExperiment::~SignalExperiment() {
	delete this->network;
}

void SignalExperiment::save(ofstream& output_file) {
	this->network->save(output_file);

	output_file << this->signal_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->signal_history.size(); h_index++) {
		output_file << this->signal_history[h_index] << endl;
	}
}
