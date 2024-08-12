#include "world_model.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "world_state.h"

using namespace std;

const int STARTING_NUM_STATES = 2;

WorldModel::WorldModel() {
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
		WorldState* world_state = new WorldState();

		world_state->average_val = distribution(generator);

		world_state->transitions = vector<vector<double>>(NUM_ACTIONS);
		for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
			vector<double> likelihoods(STARTING_NUM_STATES);

			double sum_likelihood = 0.0;
			for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
				likelihoods[s_index] = distribution(generator);

				sum_likelihood += likelihoods[s_index];
			}

			for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
				likelihoods[s_index] /= sum_likelihood;
			}

			world_state->transitions[a_index] = likelihoods;
		}

		this->states.push_back(world_state);
	}

	{
		this->starting_likelihood = vector<double>(STARTING_NUM_STATES);

		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
			this->starting_likelihood[s_index] = distribution(generator);

			sum_likelihood += this->starting_likelihood[s_index];
		}

		for (int s_index = 0; s_index < STARTING_NUM_STATES; s_index++) {
			this->starting_likelihood[s_index] /= sum_likelihood;
		}
	}
}

WorldModel::WorldModel(WorldModel* original) {
	for (int s_index = 0; s_index < (int)original->states.size(); s_index++) {
		this->states.push_back(new WorldState(original->states[s_index]));
	}

	this->starting_likelihood = original->starting_likelihood;
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
}

WorldModel::~WorldModel() {
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		delete this->states[s_index];
	}
}

void WorldModel::split_state(int state_index) {
	this->states.push_back(new WorldState(this->states[state_index]));
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		this->states[s_index]->split_state(state_index);
	}

	uniform_real_distribution<double> distribution(0.0, 1.0);

	/**
	 * - with random likelihood, some transitions now need to go to other state
	 * 
	 * TODO: though may represent disjointed states?
	 */
	{
		for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
			double original_likelihood = distribution(generator);
			double new_likelihood = distribution(generator);
			double sum_likelihood = original_likelihood + new_likelihood;

			double scale = original_likelihood / sum_likelihood;

			for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
				this->states[state_index]->transitions[a_index][s_index] *= scale;
			}

			this->states[state_index]->transitions[a_index].back() += new_likelihood / sum_likelihood;
		}
	}
	{
		for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
			double original_likelihood = distribution(generator);
			double new_likelihood = distribution(generator);
			double sum_likelihood = original_likelihood + new_likelihood;

			double scale = original_likelihood / sum_likelihood;

			for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
				this->states.back()->transitions[a_index][s_index] *= scale;
			}

			this->states.back()->transitions[a_index][state_index] += new_likelihood / sum_likelihood;
		}
	}

	{
		double original_likelihood = distribution(generator);
		double new_likelihood = distribution(generator);
		double sum_likelihood = original_likelihood + new_likelihood;

		double original_starting = this->starting_likelihood[state_index];

		this->starting_likelihood[state_index] = original_starting * original_likelihood / sum_likelihood;
		this->starting_likelihood.push_back(original_starting * new_likelihood / sum_likelihood);
	}
}

void WorldModel::add_path(int original_state_index,
						  int starting_state_index,
						  vector<int>& new_state_indexes,
						  int ending_state_index,
						  int starting_action,
						  vector<int>& actions,
						  int ending_action) {
	int new_num_states = (int)actions.size() + 1;

	for (int n_index = 0; n_index < new_num_states; n_index++) {
		new_state_indexes.push_back((int)this->states.size());
		this->states.push_back(new WorldState(this->states[original_state_index]));
	}
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		this->states[s_index]->split_state(original_state_index,
										   new_num_states);
	}

	this->states[starting_state_index]->fixed_transitions.push_back({starting_action, {new_state_indexes[0], 0.9}});
	for (int n_index = 0; n_index < (int)new_state_indexes.size()-1; n_index++) {
		this->states[new_state_indexes[n_index]]->fixed_transitions.push_back({actions[n_index], {new_state_indexes[n_index+1], 0.9}});
	}
	this->states[new_state_indexes.back()]->fixed_transitions.push_back({ending_action, {ending_state_index, 0.9}});

	uniform_real_distribution<double> distribution(0.0, 1.0);

	{
		double sum_likelihood = 0.0;

		double original_likelihood = distribution(generator);
		sum_likelihood += original_likelihood;

		vector<double> new_likelihoods(new_num_states);
		for (int s_index = 0; s_index < new_num_states; s_index++) {
			new_likelihoods[s_index] = distribution(generator);

			sum_likelihood += new_likelihoods[s_index];
		}

		double original_starting = this->starting_likelihood[original_state_index];

		this->starting_likelihood[original_state_index] = original_starting * original_likelihood / sum_likelihood;

		for (int s_index = 0; s_index < new_num_states; s_index++) {
			this->starting_likelihood.push_back(original_starting * new_likelihoods[s_index] / sum_likelihood);
		}
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
}

void WorldModel::save_for_display(ofstream& output_file) {
	output_file << this->states.size() << endl;
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		this->states[s_index]->save_for_display(output_file);
	}
}
