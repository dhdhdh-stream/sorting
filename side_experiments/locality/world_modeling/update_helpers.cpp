#include "update_helpers.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "world_model.h"
#include "world_state.h"
#include "world_truth.h"

using namespace std;

const int UPDATE_NUM_RUNS = 10;

void process_run(WorldModel* world_model,
				 vector<double>& obs,
				 vector<int>& actions,
				 vector<vector<vector<double>>>& sum_transitions,
				 vector<double>& sum_obs,
				 vector<double>& sum_counts,
				 vector<double>& sum_starting) {
	vector<vector<double>> forward_likelihoods;
	forward_likelihoods.push_back(world_model->starting_likelihood);

	vector<double> forward_unknown_likelihoods;
	forward_unknown_likelihoods.push_back(0.0);

	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		vector<double> next_likelihoods(world_model->states.size(), 0.0);
		double next_unknown_likelihood = 0.0;

		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			world_model->states[s_index]->forward(obs[o_index],
												  forward_likelihoods.back(),
												  actions[o_index],
												  next_likelihoods,
												  next_unknown_likelihood);
		}
		{
			double unknown_state_likelihood = forward_unknown_likelihoods.back();
			next_unknown_likelihood += unknown_state_likelihood;
		}

		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			sum_likelihood += next_likelihoods[s_index];
		}
		sum_likelihood += next_unknown_likelihood;

		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			next_likelihoods[s_index] /= sum_likelihood;
		}
		next_unknown_likelihood /= sum_likelihood;

		forward_likelihoods.push_back(next_likelihoods);
		forward_unknown_likelihoods.push_back(next_unknown_likelihood);
	}

	vector<vector<double>> backward_likelihoods;
	backward_likelihoods.insert(backward_likelihoods.begin(), vector<double>(world_model->states.size(), 1.0));

	vector<double> backward_unknown_likelihoods;
	backward_unknown_likelihoods.insert(backward_unknown_likelihoods.begin(), 1.0);

	for (int o_index = (int)obs.size()-1; o_index >= 0; o_index--) {
		vector<double> next_likelihoods(world_model->states.size(), 0.0);
		double next_unknown_likelihood = 0.0;

		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			world_model->states[s_index]->backward(backward_likelihoods[0],
												   backward_unknown_likelihoods[0],
												   actions[o_index],
												   obs[o_index],
												   next_likelihoods);
		}
		{
			double unknown_state_likelihood = backward_unknown_likelihoods[0];
			next_unknown_likelihood += unknown_state_likelihood
				* 0.5;
		}

		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			sum_likelihood += next_likelihoods[s_index];
		}
		sum_likelihood += next_unknown_likelihood;

		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			next_likelihoods[s_index] /= sum_likelihood;
		}
		next_unknown_likelihood /= sum_likelihood;

		backward_likelihoods.insert(backward_likelihoods.begin(), next_likelihoods);
		backward_unknown_likelihoods.insert(backward_unknown_likelihoods.begin(), next_unknown_likelihood);
	}

	vector<vector<double>> state_probabilities(obs.size() + 1);
	for (int o_index = 0; o_index < (int)obs.size() + 1; o_index++) {
		vector<double> curr_likelihood(world_model->states.size());
		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			curr_likelihood[s_index] = forward_likelihoods[o_index][s_index] * backward_likelihoods[o_index][s_index];

			sum_likelihood += curr_likelihood[s_index];
		}

		vector<double> curr_probabilities(world_model->states.size());
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			if (sum_likelihood == 0.0) {
				curr_probabilities[s_index] = 0.0;
			} else {
				curr_probabilities[s_index] = curr_likelihood[s_index] / sum_likelihood;
			}
		}
		state_probabilities[o_index] = curr_probabilities;
	}

	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			if (state_probabilities[o_index][s_index] > 0.0) {
				double sum_likelihood = 0.0;
				for (int t_index = 0; t_index < (int)world_model->states[s_index]->transitions[actions[o_index]].size(); t_index++) {
					int end_state_index = world_model->states[s_index]->transitions[actions[o_index]][t_index].first->id;
					sum_likelihood += state_probabilities[o_index+1][end_state_index];
				}

				for (int t_index = 0; t_index < (int)world_model->states[s_index]->transitions[actions[o_index]].size(); t_index++) {
					int end_state_index = world_model->states[s_index]->transitions[actions[o_index]][t_index].first->id;
					sum_transitions[actions[o_index]][s_index][t_index]
						+= state_probabilities[o_index+1][end_state_index]
							/ sum_likelihood
							* state_probabilities[o_index][s_index];
				}
			}
		}
	}

	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			sum_obs[s_index] += state_probabilities[o_index][s_index] * obs[o_index];

			sum_counts[s_index] += state_probabilities[o_index][s_index];
		}
	}

	for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
		sum_starting[s_index] += state_probabilities[0][s_index];
	}
}

void update(WorldModel* world_model,
			vector<vector<double>>& obs,
			vector<vector<int>>& actions) {
	for (int iter_index = 0; iter_index < UPDATE_NUM_RUNS; iter_index++) {
		vector<vector<vector<double>>> sum_transitions(NUM_ACTIONS);
		for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
			sum_transitions[a_index] = vector<vector<double>>(world_model->states.size());
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_transitions[a_index][s_index] = vector<double>(
					world_model->states[s_index]->transitions[a_index].size(), 0.0);
			}
		}

		vector<double> sum_obs(world_model->states.size(), 0.0);
		vector<double> sum_counts(world_model->states.size(), 0.0);

		vector<double> sum_starting(world_model->states.size(), 0.0);

		for (int r_index = 0; r_index < (int)obs.size(); r_index++) {
			process_run(world_model,
						obs[r_index],
						actions[r_index],
						sum_transitions,
						sum_obs,
						sum_counts,
						sum_starting);
		}

		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
				double sum_likelihood = 0.0;
				for (int t_index = 0; t_index < (int)world_model->states[s_index]->transitions[a_index].size(); t_index++) {
					sum_likelihood += sum_transitions[a_index][s_index][t_index];
				}
				/**
				 * - in case state + action combination never hit(?)
				 */
				if (sum_likelihood != 0.0) {
					for (int t_index = 0; t_index < (int)world_model->states[s_index]->transitions[a_index].size(); t_index++) {
						world_model->states[s_index]->transitions[a_index][t_index].second
							= sum_transitions[a_index][s_index][t_index] / sum_likelihood;

						if (abs(world_model->states[s_index]->transitions[a_index][t_index].second) < MIN_WEIGHT) {
							world_model->states[s_index]->transitions[a_index][t_index].second = 0.0;
						}
					}
				}
			}
		}

		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			if (sum_counts[s_index] != 0.0) {
				world_model->states[s_index]->average_val = sum_obs[s_index] / sum_counts[s_index];
			}
		}

		{
			double sum_likelihood = 0.0;
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_likelihood += sum_starting[s_index];
			}

			/**
			 * - if sequence too long and unknown dominates, then sum_likelihood can effectively become 0.0
			 *   - if so, leave starting as-is
			 */
			if (sum_likelihood != 0.0) {
				/**
				 * - always leave states[0] with a minimum starting_likelihood percentage
				 */
				if (sum_starting[0] / sum_likelihood < 1.0 / world_model->states.size()) {
					double added_likelihood = sum_likelihood / world_model->states.size();
					sum_starting[0] += added_likelihood;
					sum_likelihood += added_likelihood;
				}

				for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
					world_model->starting_likelihood[s_index] = sum_starting[s_index] / sum_likelihood;

					if (abs(world_model->starting_likelihood[s_index]) < MIN_WEIGHT) {
						world_model->starting_likelihood[s_index] = 0.0;
					}
				}
			}
		}
	}
}
