#include "world_state.h"

#include "constants.h"
#include "globals.h"

using namespace std;

WorldState::WorldState() {
	// do nothing
}

WorldState::WorldState(WorldState* original) {
	this->average_val = original->average_val;

	this->transitions = original->transitions;

	this->fixed_transitions = original->fixed_transitions;
}

WorldState::WorldState(ifstream& input_file) {
	string average_val_line;
	getline(input_file, average_val_line);
	this->average_val = stod(average_val_line);

	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);

	this->transitions = vector<vector<double>>(NUM_ACTIONS);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		this->transitions[a_index] = vector<double>(num_states);
		for (int s_index = 0; s_index < num_states; s_index++) {
			string likelihood_line;
			getline(input_file, likelihood_line);
			this->transitions[a_index][s_index] = stod(likelihood_line);
		}
	}

	string num_fixed_line;
	getline(input_file, num_fixed_line);
	int num_fixed = stoi(num_fixed_line);
	for (int f_index = 0; f_index < num_fixed; f_index++) {
		string action_line;
		getline(input_file, action_line);
		int action = stoi(action_line);

		string state_index_line;
		getline(input_file, state_index_line);
		int state_index = stoi(state_index_line);

		string likelihood_line;
		getline(input_file, likelihood_line);
		double likelihood = stod(likelihood_line);

		this->fixed_transitions.push_back({action, {state_index, likelihood}});
	}
}

void WorldState::split_state(int state_index) {
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		double original_likelihood = distribution(generator);
		double new_likelihood = distribution(generator);
		double sum_likelihood = original_likelihood + new_likelihood;

		double original_transition = this->transitions[a_index][state_index];

		this->transitions[a_index][state_index] = original_transition * original_likelihood / sum_likelihood;
		this->transitions[a_index].push_back(original_transition * new_likelihood / sum_likelihood);
	}
}

void WorldState::split_state(int state_index,
							 int num_split) {
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		double sum_likelihood = 0.0;

		double original_likelihood = distribution(generator);
		sum_likelihood += original_likelihood;

		vector<double> new_likelihoods(num_split);
		for (int s_index = 0; s_index < num_split; s_index++) {
			new_likelihoods[s_index] = distribution(generator);

			sum_likelihood += new_likelihoods[s_index];
		}

		double original_transition = this->transitions[a_index][state_index];

		this->transitions[a_index][state_index] = original_transition * original_likelihood / sum_likelihood;

		for (int s_index = 0; s_index < num_split; s_index++) {
			this->transitions[a_index].push_back(original_transition * new_likelihoods[s_index] / sum_likelihood);
		}
	}
}

void WorldState::apply_fixed() {
	for (int f_index = 0; f_index < (int)this->fixed_transitions.size(); f_index++) {
		int action = this->fixed_transitions[f_index].first;
		double scale = 1.0 - this->fixed_transitions[f_index].second.second;
		for (int s_index = 0; s_index < (int)this->transitions[action].size(); s_index++) {
			this->transitions[action][s_index] *= scale;
		}

		this->transitions[action][this->fixed_transitions[f_index].second.first] +=
			this->fixed_transitions[f_index].second.second;
	}
}

void WorldState::save(ofstream& output_file) {
	output_file << this->average_val << endl;

	output_file << this->transitions[0].size() << endl;

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		for (int s_index = 0; s_index < (int)this->transitions[a_index].size(); s_index++) {
			output_file << this->transitions[a_index][s_index] << endl;
		}
	}

	output_file << this->fixed_transitions.size() << endl;
	for (int f_index = 0; f_index < (int)this->fixed_transitions.size(); f_index++) {
		output_file << this->fixed_transitions[f_index].first << endl;
		output_file << this->fixed_transitions[f_index].second.first << endl;
		output_file << this->fixed_transitions[f_index].second.second << endl;
	}
}

void WorldState::save_for_display(ofstream& output_file) {
	output_file << this->average_val << endl;

	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		for (int s_index = 0; s_index < (int)this->transitions[a_index].size(); s_index++) {
			output_file << this->transitions[a_index][s_index] << endl;
		}
	}
}
