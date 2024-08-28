#include "key_point.h"

#include <iostream>

#include "world_truth.h"

using namespace std;

KeyPoint::KeyPoint(vector<double> obs,
				   vector<int> actions) {
	this->obs = obs;
	this->actions = actions;
}

bool KeyPoint::match(WorldTruth* world_truth) {
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

bool KeyPoint::match(WorldTruth* world_truth,
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
