/**
 * - updating average_val makes things converge
 *   - with each cycle, the value gets sharper and sharper
 *     - pushing the uncertainty into the transitions
 * 
 * - but if any transitions missing, then average_val will carry the uncertainty
 */

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
				 vector<double>& sum_vals,
				 vector<double>& sum_variances,
				 vector<double>& sum_counts,
				 vector<double>& sum_starting) {
	vector<vector<double>> forward_likelihoods;
	forward_likelihoods.push_back(world_model->starting_likelihood);
	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		vector<double> next_likelihoods(world_model->states.size(), 0.0);
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			world_model->states[s_index]->forward(obs[o_index],
												  forward_likelihoods.back(),
												  actions[o_index],
												  next_likelihoods);
		}

		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			sum_likelihood += next_likelihoods[s_index];
		}
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			next_likelihoods[s_index] /= sum_likelihood;
		}

		forward_likelihoods.push_back(next_likelihoods);
	}

	vector<vector<double>> backward_likelihoods;
	backward_likelihoods.insert(backward_likelihoods.begin(), vector<double>(world_model->states.size(), 1.0));
	for (int o_index = (int)obs.size()-1; o_index >= 0; o_index--) {
		vector<double> next_likelihoods(world_model->states.size(), 0.0);
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			world_model->states[s_index]->backward(backward_likelihoods[0],
												   actions[o_index],
												   obs[o_index],
												   next_likelihoods);
		}

		double sum_likelihood = 0.0;
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			sum_likelihood += next_likelihoods[s_index];
		}
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			next_likelihoods[s_index] /= sum_likelihood;
		}

		backward_likelihoods.insert(backward_likelihoods.begin(), next_likelihoods);
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
			curr_probabilities[s_index] = curr_likelihood[s_index] / sum_likelihood;
		}
		state_probabilities[o_index] = curr_probabilities;
	}

	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			double sum_likelihood = 0.0;
			for (int t_index = 0; t_index < (int)world_model->states[s_index]->transitions[actions[o_index]].size(); t_index++) {
				int end_state_index = world_model->states[s_index]->transitions[actions[o_index]][t_index].first;
				sum_likelihood += state_probabilities[o_index+1][end_state_index];
			}

			if (sum_likelihood > 0.0) {
				for (int t_index = 0; t_index < (int)world_model->states[s_index]->transitions[actions[o_index]].size(); t_index++) {
					int end_state_index = world_model->states[s_index]->transitions[actions[o_index]][t_index].first;
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
			sum_vals[s_index] += state_probabilities[o_index][s_index] * obs[o_index];

			sum_variances[s_index] += state_probabilities[o_index][s_index] *
				(world_model->states[s_index]->average_val - obs[o_index]) * (world_model->states[s_index]->average_val - obs[o_index]);

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

		vector<double> sum_vals(world_model->states.size(), 0.0);
		vector<double> sum_variances(world_model->states.size(), 0.0);
		vector<double> sum_counts(world_model->states.size(), 0.0);

		vector<double> sum_starting(world_model->states.size(), 0.0);

		for (int r_index = 0; r_index < (int)obs.size(); r_index++) {
			process_run(world_model,
						obs[r_index],
						actions[r_index],
						sum_transitions,
						sum_vals,
						sum_variances,
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
				world_model->states[s_index]->average_val = sum_vals[s_index] / sum_counts[s_index];
				if (abs(world_model->states[s_index]->average_val) < MIN_WEIGHT) {
					world_model->states[s_index]->average_val = 0.0;
				}

				world_model->states[s_index]->average_standard_deviation =
					sqrt(sum_variances[s_index] / sum_counts[s_index]);
				if (abs(world_model->states[s_index]->average_standard_deviation) < MIN_WEIGHT) {
					world_model->states[s_index]->average_standard_deviation = 0.0;
				}
			}
		}

		{
			double sum_likelihood = 0.0;
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_likelihood += sum_starting[s_index];
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
