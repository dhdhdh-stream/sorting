#include "localize_helpers.h"

#include <iostream>
#include <vector>

#include "constants.h"
#include "utilities.h"
#include "world_model.h"
#include "world_state.h"
#include "world_truth.h"

using namespace std;

const int LOCALIZE_STEPS = 5;
const double SUCCESS_PROBABILITY = 0.7;

bool localize_helper(WorldTruth* world_truth,
					 WorldModel* world_model,
					 vector<vector<double>>& likelihoods,
					 vector<double>& obs,
					 vector<int>& actions) {
	int highest_state_index = -1;
	double highest_state_likelihood = 0.0;
	for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
		if (likelihoods.back()[s_index] > highest_state_likelihood) {
			highest_state_index = s_index;
			highest_state_likelihood = likelihoods.back()[s_index];
		}
	}

	int highest_action = -1;
	double highest_action_likelihood = 0.0;
	{
		WorldState* state = world_model->states[highest_state_index];
		for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
			for (int t_index = 0; t_index < (int)state->transitions[a_index].size(); t_index++) {
				if (state->transitions[a_index][t_index].second > highest_action_likelihood) {
					highest_action = a_index;
					highest_action_likelihood = state->transitions[a_index][t_index].second;
				}
			}
		}
	}

	actions.push_back(highest_action);

	world_truth->move(highest_action);

	double curr_obs = world_truth->get_obs();
	obs.push_back(curr_obs);

	vector<double> next_likelihoods(world_model->states.size());
	for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
		world_model->states[s_index]->transition_step(
			likelihoods.back(),
			highest_action,
			next_likelihoods);
	}
	for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
		world_model->states[s_index]->obs_step(
			next_likelihoods,
			curr_obs);
	}

	double sum_likelihood = 0.0;
	for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
		sum_likelihood += next_likelihoods[s_index];
	}
	if (sum_likelihood == 0.0) {
		return false;
	}
	for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
		next_likelihoods[s_index] /= sum_likelihood;
	}

	likelihoods.push_back(next_likelihoods);

	return true;
}

bool localize(WorldTruth* world_truth,
			  WorldModel* world_model,
			  vector<int>& unknown_actions) {
	vector<vector<double>> likelihoods;

	vector<double> obs;
	vector<int> actions;

	double starting_obs = world_truth->get_obs();
	obs.push_back(starting_obs);

	{
		vector<double> starting_likelihood(world_model->states.size());
		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			WorldState* state = world_model->states[s_index];
			if (state->average_standard_deviation == 0.0) {
				if (starting_obs == state->average_val) {
					starting_likelihood[s_index] = 1.0;
				} else {
					starting_likelihood[s_index] = 0.0;
				}
			} else {
				double z_score = -abs((starting_obs - state->average_val) / state->average_standard_deviation);
				starting_likelihood[s_index] = 2.0 * cumulative_normal(z_score);
			}

			sum_likelihood += starting_likelihood[s_index];
		}
		if (sum_likelihood == 0.0) {
			return false;
		}
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			starting_likelihood[s_index] /= sum_likelihood;
		}
		likelihoods.push_back(starting_likelihood);
	}

	for (int i_index = 0; i_index < LOCALIZE_STEPS; i_index++) {
		bool can_continue = localize_helper(
			world_truth,
			world_model,
			likelihoods,
			obs,
			actions);
		if (!can_continue) {
			unknown_actions.insert(unknown_actions.end(),
				actions.begin(), actions.end());
			return false;
		}
	}

	vector<int> most_likely_path;

	{
		int highest_state_index = -1;
		double highest_state_likelihood = 0.0;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			if (likelihoods.back()[s_index] > highest_state_likelihood) {
				highest_state_index = s_index;
				highest_state_likelihood = likelihoods.back()[s_index];
			}
		}

		most_likely_path.insert(most_likely_path.begin(), highest_state_index);
	}

	for (int i_index = likelihoods.size() - 2; i_index >= 0; i_index--) {
		int highest_state_index = -1;
		double highest_state_likelihood = 0.0;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			WorldState* state = world_model->states[s_index];
			for (int t_index = 0; t_index < (int)state->transitions[actions[i_index]].size(); t_index++) {
				if (state->transitions[actions[i_index]][t_index].first == most_likely_path[0]) {
					double transition_likelihood = state->transitions[actions[i_index]][t_index].second;
					double likelihood = likelihoods[i_index][s_index] * transition_likelihood;
					if (likelihood > highest_state_likelihood) {
						highest_state_index = s_index;
						highest_state_likelihood = likelihood;
					}
				}
			}
		}

		most_likely_path.insert(most_likely_path.begin(), highest_state_index);
	}

	cout << "most_likely_path:";
	for (int i_index = 0; i_index < (int)most_likely_path.size(); i_index++) {
		cout << " " << most_likely_path[i_index];
	}
	cout << endl;

	vector<double> path_likelihoods;
	for (int i_index = 0; i_index < (int)most_likely_path.size(); i_index++) {
		WorldState* state = world_model->states[most_likely_path[i_index]];

		double obs_likelihood;
		if (state->average_standard_deviation == 0.0) {
			if (obs[i_index] == state->average_val) {
				obs_likelihood = 1.0;
			} else {
				obs_likelihood = 0.0;
			}
		} else {
			double z_score = -abs((obs[i_index] - state->average_val) / state->average_standard_deviation);
			obs_likelihood = 2.0 * cumulative_normal(z_score);
		}

		path_likelihoods.push_back(obs_likelihood);

		if (i_index != (int)most_likely_path.size()-1) {
			for (int t_index = 0; t_index < (int)state->transitions[actions[i_index]].size(); t_index++) {
				if (state->transitions[actions[i_index]][t_index].first == most_likely_path[i_index+1]) {
					path_likelihoods.push_back(state->transitions[actions[i_index]][t_index].second);
					break;
				}
			}
		}
	}

	double sum_likelihoods = 0.0;
	for (int l_index = 0; l_index < (int)path_likelihoods.size(); l_index++) {
		sum_likelihoods += path_likelihoods[l_index];
	}
	double average_likelihood = sum_likelihoods / path_likelihoods.size();

	cout << "average_likelihood: " << average_likelihood << endl;

	if (average_likelihood > SUCCESS_PROBABILITY) {
		return true;
	} else {
		unknown_actions.insert(unknown_actions.end(),
			actions.begin(), actions.end());
		return false;
	}
}
