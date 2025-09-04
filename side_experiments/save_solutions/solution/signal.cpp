#include "signal.h"

#include "signal_instance.h"

using namespace std;

Signal::Signal() {
	// do nothing
}

Signal::Signal(ifstream& input_file) {
	string num_pre_actions_line;
	getline(input_file, num_pre_actions_line);
	int num_pre_actions = stoi(num_pre_actions_line);
	for (int a_index = 0; a_index < num_pre_actions; a_index++) {
		string action_line;
		getline(input_file, action_line);
		this->signal_pre_actions.push_back(stoi(action_line));
	}

	string num_post_actions_line;
	getline(input_file, num_post_actions_line);
	int num_post_actions = stoi(num_post_actions_line);
	for (int a_index = 0; a_index < num_post_actions; a_index++) {
		string action_line;
		getline(input_file, action_line);
		this->signal_post_actions.push_back(stoi(action_line));
	}

	string num_signals_line;
	getline(input_file, num_signals_line);
	int num_signals = stoi(num_signals_line);
	for (int s_index = 0; s_index < num_signals; s_index++) {
		this->instances.push_back(new SignalInstance(input_file));
	}

	string default_guess_line;
	getline(input_file, default_guess_line);
	this->default_guess = stod(default_guess_line);

	string signal_positive_misguess_average_line;
	getline(input_file, signal_positive_misguess_average_line);
	this->signal_positive_misguess_average = stod(signal_positive_misguess_average_line);

	string signal_positive_misguess_standard_deviation_line;
	getline(input_file, signal_positive_misguess_standard_deviation_line);
	this->signal_positive_misguess_standard_deviation = stod(signal_positive_misguess_standard_deviation_line);

	string signal_misguess_average_line;
	getline(input_file, signal_misguess_average_line);
	this->signal_misguess_average = stod(signal_misguess_average_line);

	string signal_misguess_standard_deviation_line;
	getline(input_file, signal_misguess_standard_deviation_line);
	this->signal_misguess_standard_deviation = stod(signal_misguess_standard_deviation_line);
}

Signal::~Signal() {
	for (int i_index = 0; i_index < (int)this->instances.size(); i_index++) {
		delete this->instances[i_index];
	}
}

void Signal::save(ofstream& output_file) {
	output_file << this->signal_pre_actions.size() << endl;
	for (int a_index = 0; a_index < (int)this->signal_pre_actions.size(); a_index++) {
		output_file << this->signal_pre_actions[a_index] << endl;
	}

	output_file << this->signal_post_actions.size() << endl;
	for (int a_index = 0; a_index < (int)this->signal_post_actions.size(); a_index++) {
		output_file << this->signal_post_actions[a_index] << endl;
	}

	output_file << this->instances.size() << endl;
	for (int s_index = 0; s_index < (int)this->instances.size(); s_index++) {
		this->instances[s_index]->save(output_file);
	}

	output_file << this->default_guess << endl;

	output_file << this->signal_positive_misguess_average << endl;
	output_file << this->signal_positive_misguess_standard_deviation << endl;
	output_file << this->signal_misguess_average << endl;
	output_file << this->signal_misguess_standard_deviation << endl;
}
