#include "update_helpers.h"

#include <cmath>

#include "constants.h"
#include "world_model.h"
#include "world_state.h"
#include "world_truth.h"

using namespace std;

/**
 * - WorldModel gradually built up, so don't need as many update runs?
 */
// const int UPDATE_NUM_RUNS = 30;
const int UPDATE_NUM_RUNS = 50;

void process_run(WorldModel* world_model,
				 vector<double>& obs,
				 vector<int>& actions,
				 vector<vector<vector<double>>>& sum_transitions,
				 vector<double>& sum_obs,
				 vector<double>& sum_counts,
				 vector<double>& sum_starting) {
	vector<vector<double>> forward_likelihoods;

	forward_likelihoods.push_back(world_model->starting_likelihood);

	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		vector<vector<double>> curr_likelihood(world_model->states.size());
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			curr_likelihood[s_index] = vector<double>(world_model->states.size());
		}

		double sum_likelihood = 0.0;
		for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
			double likelihood = forward_likelihoods.back()[start_index]
				* (1.0 - abs(obs[o_index] - world_model->states[start_index]->average_val));
			for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
				curr_likelihood[start_index][end_index] = likelihood
					* world_model->states[start_index]->transitions[actions[o_index]][end_index];

				sum_likelihood += curr_likelihood[start_index][end_index];
			}
		}

		vector<double> next_likelihood(world_model->states.size());
		for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
			double sum_end_likelihood = 0.0;
			for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
				sum_end_likelihood += curr_likelihood[start_index][end_index];
			}
			next_likelihood[end_index] = sum_end_likelihood / sum_likelihood;
		}

		forward_likelihoods.push_back(next_likelihood);
	}

	vector<vector<double>> backward_likelihoods;

	backward_likelihoods.insert(backward_likelihoods.begin(), vector<double>(world_model->states.size(), 1.0));

	for (int o_index = (int)obs.size()-1; o_index >= 0; o_index--) {
		vector<vector<double>> curr_likelihood(world_model->states.size());
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			curr_likelihood[s_index] = vector<double>(world_model->states.size());
		}

		double sum_likelihood = 0.0;
		for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
			for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
				curr_likelihood[start_index][end_index] = backward_likelihoods[0][end_index]
					* (1.0 - abs(obs[o_index] - world_model->states[start_index]->average_val))
					* world_model->states[start_index]->transitions[actions[o_index]][end_index];

				sum_likelihood += curr_likelihood[start_index][end_index];
			}
		}

		vector<double> previous_likelihood(world_model->states.size());
		for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
			double sum_start_likelihood = 0.0;
			for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
				sum_start_likelihood += curr_likelihood[start_index][end_index];
			}
			previous_likelihood[start_index] = sum_start_likelihood / sum_likelihood;
		}

		backward_likelihoods.insert(backward_likelihoods.begin(), previous_likelihood);
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
		vector<vector<double>> curr_likelihood(world_model->states.size());
		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			curr_likelihood[s_index] = vector<double>(world_model->states.size());
		}

		double curr_sum = 0.0;
		for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
			for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
				curr_likelihood[start_index][end_index] = state_probabilities[o_index][start_index] * state_probabilities[o_index+1][end_index];

				curr_sum += curr_likelihood[start_index][end_index];
			}
		}

		for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
			for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
				sum_transitions[actions[o_index]][start_index][end_index] += curr_likelihood[start_index][end_index] / curr_sum;
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
		vector<vector<vector<double>>> sum_transitions(4);
		for (int a_index = 0; a_index < 4; a_index++) {
			sum_transitions[a_index] = vector<vector<double>>(world_model->states.size());
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_transitions[a_index][s_index] = vector<double>(world_model->states.size(), 0.0);
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

		for (int start_index = 0; start_index < (int)world_model->states.size(); start_index++) {
			for (int a_index = 0; a_index < 4; a_index++) {
				double sum_start = 0.0;
				for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
					sum_start += sum_transitions[a_index][start_index][end_index];
				}

				for (int end_index = 0; end_index < (int)world_model->states.size(); end_index++) {
					world_model->states[start_index]->transitions[a_index][end_index]
						= sum_transitions[a_index][start_index][end_index] / sum_start;
				}
			}
		}

		for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
			world_model->states[s_index]->average_val = sum_obs[s_index] / sum_counts[s_index];
		}

		{
			double sum_likelihood = 0.0;
			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				sum_likelihood += sum_starting[s_index];
			}

			for (int s_index = 0; s_index < (int)world_model->states.size(); s_index++) {
				world_model->starting_likelihood[s_index] = sum_starting[s_index] / sum_likelihood;
			}
		}
	}
}
