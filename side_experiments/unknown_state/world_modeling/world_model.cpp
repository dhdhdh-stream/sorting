#include "world_model.h"

#include "constants.h"
#include "globals.h"
#include "world_state.h"

using namespace std;

WorldModel::WorldModel() {
	WorldState* initial_world_state = new WorldState();

	initial_world_state->average_val = 0.5;

	initial_world_state->transitions = vector<vector<double>>(NUM_ACTIONS);
	initial_world_state->unknown_transitions = vector<double>(NUM_ACTIONS);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		initial_world_state->transitions[a_index].push_back(0.5);

		initial_world_state->unknown_transitions[a_index] = 0.5;
	}

	this->states.push_back(initial_world_state);

	this->starting_likelihood = vector<double>{1.0};

	this->curr_unknown_misguess = 0.5;
}

WorldModel::WorldModel(WorldModel* original) {
	for (int s_index = 0; s_index < (int)original->states.size(); s_index++) {
		this->states.push_back(new WorldState(original->states[s_index]));
	}

	this->starting_likelihood = original->starting_likelihood;

	this->curr_unknown_misguess = original->curr_unknown_misguess;
}

WorldModel::WorldModel(ifstream& input_file) {
	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);

	for (int s_index = 0; s_index < num_states; s_index++) {
		this->states.push_back(new WorldState(input_file));
	}

	for (int s_index = 0; s_index < num_states; s_index++) {
		string likelihood_line;
		getline(input_file, likelihood_line);
		this->starting_likelihood.push_back(stod(likelihood_line));
	}

	string curr_unknown_misguess_line;
	getline(input_file, curr_unknown_misguess_line);
	this->curr_unknown_misguess = stod(curr_unknown_misguess_line);
}

WorldModel::~WorldModel() {
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		delete this->states[s_index];
	}
}

void WorldModel::add_state(int seed_state_index,
						   int seed_action) {
	int previous_num_states = (int)this->states.size();

	for (int s_index = 0; s_index < previous_num_states; s_index++) {
		this->states[s_index]->add_state();
	}
	{
		double unscale = (previous_num_states + 1) / previous_num_states;
		double scale = unscale * 0.1;

		for (int s_index = 0; s_index < previous_num_states; s_index++) {
			this->states[seed_state_index]->transitions[seed_action][s_index] *= scale;
		}

		this->states[seed_state_index]->unknown_transitions[seed_action] *= scale;

		this->states[seed_state_index]->transitions[seed_action].back() = 0.9;
	}

	WorldState* new_world_state = new WorldState();
	this->states.push_back(new_world_state);

	uniform_real_distribution<double> distribution(0.0, 1.0);

	new_world_state->average_val = distribution(generator);

	int new_num_states = (int)this->states.size();

	new_world_state->transitions = vector<vector<double>>(NUM_ACTIONS);
	new_world_state->unknown_transitions = vector<double>(NUM_ACTIONS);
	for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
		vector<double> likelihoods(new_num_states);
		double unknown_likelihood;

		double sum_likelihood = 0.0;

		for (int s_index = 0; s_index < new_num_states; s_index++) {
			likelihoods[s_index] = distribution(generator);

			sum_likelihood += likelihoods[s_index];
		}

		{
			unknown_likelihood = distribution(generator);

			sum_likelihood += unknown_likelihood;
		}

		for (int s_index = 0; s_index < new_num_states; s_index++) {
			likelihoods[s_index] /= sum_likelihood;
		}

		unknown_likelihood /= sum_likelihood;

		new_world_state->transitions[a_index] = likelihoods;
		new_world_state->unknown_transitions[a_index] = unknown_likelihood;
	}

	{
		double scale = (double)(new_num_states - 1) / (double)new_num_states;

		for (int s_index = 0; s_index < (int)this->starting_likelihood.size(); s_index++) {
			this->starting_likelihood[s_index] *= scale;
		}

		this->starting_likelihood.push_back(1.0 / (double)new_num_states);
	}
}

void WorldModel::save(ofstream& output_file) {
	output_file << this->states.size() << endl;

	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		this->states[s_index]->save(output_file);
	}

	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		output_file << this->starting_likelihood[s_index] << endl;
	}

	output_file << this->curr_unknown_misguess << endl;
}

void WorldModel::save_for_display(ofstream& output_file) {
	output_file << this->states.size() << endl;
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		this->states[s_index]->save_for_display(output_file);
	}
}
