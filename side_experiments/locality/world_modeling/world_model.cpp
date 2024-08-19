#include "world_model.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "world_state.h"

using namespace std;

WorldModel::WorldModel() {
	// do nothing
}

WorldModel::WorldModel(WorldModel* original) {
	for (int s_index = 0; s_index < (int)original->states.size(); s_index++) {
		WorldState* world_state = new WorldState();
		world_state->parent = this;
		world_state->id = s_index;
		this->states.push_back(world_state);
	}
	for (int s_index = 0; s_index < (int)original->states.size(); s_index++) {
		this->states[s_index]->copy_from(original->states[s_index]);
	}

	this->starting_likelihood = original->starting_likelihood;
}

WorldModel::WorldModel(ifstream& input_file) {
	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);

	for (int s_index = 0; s_index < num_states; s_index++) {
		WorldState* world_state = new WorldState();
		world_state->parent = this;
		world_state->id = s_index;
		this->states.push_back(world_state);
	}
	for (int s_index = 0; s_index < num_states; s_index++) {
		this->states[s_index]->load(input_file);
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

void WorldModel::init() {
	WorldState* world_state = new WorldState();
	world_state->parent = this;
	world_state->id = this->states.size();
	this->states.push_back(world_state);

	uniform_real_distribution<double> distribution(0.0, 1.0);
	world_state->average_val = distribution(generator);

	world_state->transitions = vector<vector<pair<WorldState*,double>>>(NUM_ACTIONS);

	this->starting_likelihood = vector<double>{1.0};
}

void WorldModel::clean() {
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
			for (int t_index = (int)this->states[s_index]->transitions[a_index].size()-1; t_index >= 0; t_index--) {
				if (abs(this->states[s_index]->transitions[a_index][t_index].second) < MIN_WEIGHT) {
					this->states[s_index]->transitions[a_index].erase(
						this->states[s_index]->transitions[a_index].begin() + t_index);
				}
			}
		}
	}

	vector<bool> needed(this->states.size(), false);
	needed[0] = true;
	while (true) {
		bool added = false;

		for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
			if (needed[s_index]) {
				for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
					for (int t_index = 0; t_index < (int)this->states[s_index]->transitions[a_index].size(); t_index++) {
						int state_index = this->states[s_index]->transitions[a_index][t_index].first->id;
						if (!needed[state_index]) {
							needed[state_index] = true;
							added = true;
						}
					}
				}
			}
		}

		if (!added) {
			break;
		}
	}
	for (int s_index = (int)this->states.size()-1; s_index >= 0; s_index--) {
		if (!needed[s_index]) {
			delete this->states[s_index];
			this->states.erase(this->states.begin() + s_index);
			this->starting_likelihood.erase(this->starting_likelihood.begin() + s_index);
		}
	}
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		this->states[s_index]->id = s_index;
	}
}

void WorldModel::random_change() {
	uniform_int_distribution<int> new_state_distribution(0, 9);
	uniform_int_distribution<int> remove_distribution(0, 9);
	uniform_int_distribution<int> state_distribution(0, (int)this->states.size()-1);
	uniform_int_distribution<int> action_distribution(0, NUM_ACTIONS-1);
	uniform_real_distribution<double> distribution(0.0, 1.0);
	while (true) {
		if (new_state_distribution(generator) == 0) {
			int starting_state_index = state_distribution(generator);
			int action = action_distribution(generator);

			WorldState* new_world_state = new WorldState();
			new_world_state->parent = this;
			new_world_state->id = this->states.size();
			this->states.push_back(new_world_state);

			uniform_real_distribution<double> distribution(0.0, 1.0);
			new_world_state->average_val = distribution(generator);

			new_world_state->transitions = vector<vector<pair<WorldState*,double>>>(NUM_ACTIONS);

			{
				double new_likelihood = distribution(generator);
				double scale = 1.0 - new_likelihood;

				for (int t_index = 0; t_index < (int)this->states[starting_state_index]->transitions[action].size(); t_index++) {
					this->states[starting_state_index]->transitions[action][t_index].second *= scale;
				}

				this->states[starting_state_index]->transitions[action].push_back({
					this->states[new_world_state->id],
					new_likelihood});
			}

			{
				double scale = (double)(this->states.size() - 1) / (double)this->states.size();

				for (int s_index = 0; s_index < (int)this->starting_likelihood.size(); s_index++) {
					this->starting_likelihood[s_index] *= scale;
				}

				this->starting_likelihood.push_back(1.0 / (double)this->states.size());
			}

			break;
		} else {
			if (remove_distribution(generator)) {
				int starting_state_index = state_distribution(generator);
				int action = action_distribution(generator);
				if (this->states[starting_state_index]->transitions[action].size() > 0) {
					uniform_int_distribution<int> transition_distribution(0, this->states[starting_state_index]->transitions[action].size()-1);
					int transition_index = transition_distribution(generator);

					this->states[starting_state_index]->transitions[action].erase(
						this->states[starting_state_index]->transitions[action].begin() + transition_index);

					break;
				}
			} else {
				int starting_state_index = state_distribution(generator);
				int ending_state_index = state_distribution(generator);
				int action = action_distribution(generator);

				bool has_existing = false;
				for (int t_index = 0; t_index < (int)this->states[starting_state_index]->transitions[action].size(); t_index++) {
					if (this->states[starting_state_index]->transitions[action][t_index].first->id == ending_state_index) {
						has_existing = true;
						break;
					}
				}

				if (!has_existing) {
					double new_likelihood = distribution(generator);
					double scale = 1.0 - new_likelihood;

					for (int t_index = 0; t_index < (int)this->states[starting_state_index]->transitions[action].size(); t_index++) {
						this->states[starting_state_index]->transitions[action][t_index].second *= scale;
					}

					this->states[starting_state_index]->transitions[action].push_back({
						this->states[ending_state_index], new_likelihood});

					break;
				}
			}
		}
	}

	vector<bool> needed(this->states.size(), false);
	needed[0] = true;
	while (true) {
		bool added = false;

		for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
			if (needed[s_index]) {
				for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
					for (int t_index = 0; t_index < (int)this->states[s_index]->transitions[a_index].size(); t_index++) {
						int state_index = this->states[s_index]->transitions[a_index][t_index].first->id;
						if (!needed[state_index]) {
							needed[state_index] = true;
							added = true;
						}
					}
				}
			}
		}

		if (!added) {
			break;
		}
	}
	for (int s_index = (int)this->states.size()-1; s_index >= 0; s_index--) {
		if (!needed[s_index]) {
			delete this->states[s_index];
			this->states.erase(this->states.begin() + s_index);
			this->starting_likelihood.erase(this->starting_likelihood.begin() + s_index);
		}
	}
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		this->states[s_index]->id = s_index;
	}

	/**
	 * - allow some uncertainty after changes occur
	 */
	for (int s_index = 0; s_index < (int)this->states.size(); s_index++) {
		if (this->states[s_index]->average_val == 1.0) {
			this->states[s_index]->average_val = 0.9;
		} else if (this->states[s_index]->average_val == 0.0) {
			this->states[s_index]->average_val = 0.1;
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
