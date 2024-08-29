#include "keypoint.h"

#include <iostream>

#include "world_truth.h"

using namespace std;

Keypoint::Keypoint() {
	// do nothing
}

bool Keypoint::match(WorldTruth* world_truth) {
	double starting_obs = world_truth->get_obs();
	if (this->obs[0] != starting_obs) {
		return false;
	}

	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		world_truth->move(this->actions[a_index]);

		double curr_obs = world_truth->get_obs();

		if (this->obs[1 + a_index] != curr_obs) {
			return false;
		}
	}

	return true;
}

bool Keypoint::match(WorldTruth* world_truth,
					 vector<int>& unknown_actions,
					 vector<double>& unknown_obs) {
	double starting_obs = world_truth->get_obs();
	if (this->obs[0] != starting_obs) {
		return false;
	}

	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		world_truth->move(this->actions[a_index]);

		double curr_obs = world_truth->get_obs();

		if (this->obs[1 + a_index] != curr_obs) {
			for (int ia_index = 0; ia_index <= a_index; ia_index++) {
				unknown_actions.push_back(this->actions[ia_index]);
				unknown_obs.push_back(this->obs[ia_index]);
			}
			/**
			 * - don't add last, missed obs
			 *   - will be handled by next
			 */
			return false;
		}
	}

	return true;
}

void Keypoint::save(ofstream& output_file) {
	output_file << this->obs.size() << endl;
	for (int o_index = 0; o_index < (int)this->obs.size(); o_index++) {
		output_file << this->obs[o_index] << endl;
	}

	output_file << this->actions.size() << endl;
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		output_file << this->actions[a_index] << endl;
	}
}

void Keypoint::load(ifstream& input_file) {
	string obs_size_line;
	getline(input_file, obs_size_line);
	int obs_size = stoi(obs_size_line);
	for (int o_index = 0; o_index < obs_size; o_index++) {
		string obs_line;
		getline(input_file, obs_line);
		this->obs.push_back(stoi(obs_line));
	}

	string actions_size_line;
	getline(input_file, actions_size_line);
	int actions_size = stoi(actions_size_line);
	for (int a_index = 0; a_index < actions_size; a_index++) {
		string action_line;
		getline(input_file, action_line);
		this->actions.push_back(stoi(action_line));
	}
}
