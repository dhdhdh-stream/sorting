/**
 * - allow stay and skip 1
 *   - otherwise, let average_val handle uncertainty
 */

#include "find_stable_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "update_helpers.h"
#include "world_model.h"
#include "world_state.h"
#include "world_truth.h"

using namespace std;

const int RANDOM_NUM_ACTIONS = 1000;

const int INITIAL_SEQ_ITERATIONS = 10;
const int BAUM_WELCH_NUM_RUNS = 100;
const int BAUM_WELCH_RUN_MIN_LENGTH = 10;
const double REPETITION_MAX_RATIO = 0.1;

/**
 * TODO:
 * - don't let number of states equal sequence length
 *   - e.g., should be 1 state if true corner, should be infinite if edge instead
 * - start from 1 and go up?
 */
WorldModel* find_stable(WorldTruth* world_truth) {
	vector<double> random_obs;
	uniform_int_distribution<int> random_action_distribution(0, NUM_ACTIONS-1);
	for (int a_index = 0; a_index < RANDOM_NUM_ACTIONS; a_index++) {
		random_obs.push_back(world_truth->get_obs());

		world_truth->move(random_action_distribution(generator));
	}

	double sum_vals = 0.0;
	for (int o_index = 0; o_index < RANDOM_NUM_ACTIONS; o_index++) {
		sum_vals += random_obs[o_index];
	}
	double average_val = sum_vals / RANDOM_NUM_ACTIONS;

	double sum_variance = 0.0;
	for (int o_index = 0; o_index < RANDOM_NUM_ACTIONS; o_index++) {
		sum_variance += (random_obs[o_index] - average_val) * (random_obs[o_index] - average_val);
	}
	double average_standard_deviation = sqrt(sum_variance / RANDOM_NUM_ACTIONS);

	cout << "average_val: " << average_val << endl;
	cout << "average_standard_deviation: " << average_standard_deviation << endl;

	geometric_distribution<int> seq_length_distribution(0.3);
	uniform_real_distribution<double> distribution(0.0, 1.0);
	while (true) {
		int seq_length = 1 + seq_length_distribution(generator);
		vector<int> sequence;
		for (int a_index = 0; a_index < seq_length; a_index++) {
			sequence.push_back(random_action_distribution(generator));
		}

		cout << "seq:";
		for (int a_index = 0; a_index < seq_length; a_index++) {
			cout << " " << sequence[a_index];
		}
		cout << endl;

		for (int i_index = 0; i_index < INITIAL_SEQ_ITERATIONS; i_index++) {
			for (int a_index = 0; a_index < seq_length; a_index++) {
				world_truth->move(sequence[a_index]);
			}
		}

		WorldModel* world_model = new WorldModel();
		for (int s_index = 0; s_index < seq_length; s_index++) {
			WorldState* world_state = new WorldState();
			world_state->id = s_index;
			world_model->states.push_back(world_state);

			world_state->average_val = average_val;
			world_state->average_standard_deviation = average_standard_deviation;

			world_state->transitions = vector<vector<pair<int,double>>>(NUM_ACTIONS);
		}

		for (int s_index = 0; s_index < seq_length; s_index++) {
			WorldState* state = world_model->states[s_index];

			int prev_index = (s_index-1+seq_length)%seq_length;
			int next_index = (s_index+1)%seq_length;
			int skip_index = (s_index+2)%seq_length;

			// arrived early
			{
				state->transitions[sequence[prev_index]].push_back({s_index, 0.1});
				state->transitions[sequence[prev_index]].push_back({next_index, 0.9});
			}

			{
				state->transitions[sequence[s_index]].push_back({s_index, 0.1});
				state->transitions[sequence[s_index]].push_back({next_index, 0.8});
				state->transitions[sequence[s_index]].push_back({skip_index, 0.1});
			}

			// stayed extra
			{
				state->transitions[sequence[next_index]].push_back({next_index, 0.9});
				state->transitions[sequence[next_index]].push_back({skip_index, 0.1});
			}

			// eliminate dupes
			for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
				for (int es_index = (int)state->transitions[a_index].size()-1; es_index >= 0; es_index--) {
					for (int ss_index = 0; ss_index < es_index; ss_index++) {
						if (state->transitions[a_index][ss_index].first == state->transitions[a_index][es_index].first) {
							state->transitions[a_index][ss_index].second += state->transitions[a_index][es_index].second;

							state->transitions[a_index].erase(state->transitions[a_index].begin() + es_index);

							break;
						}
					}
				}
			}
		}

		world_model->starting_likelihood = vector<double>(seq_length, 0.0);
		world_model->starting_likelihood[0] = 1.0;

		int num_seq_per_run = 2;
		int sum_length = 2 * seq_length;
		while (sum_length < BAUM_WELCH_RUN_MIN_LENGTH) {
			sum_length += seq_length;
			num_seq_per_run++;
		}

		vector<vector<double>> obs(BAUM_WELCH_NUM_RUNS);
		vector<vector<int>> actions(BAUM_WELCH_NUM_RUNS);
		for (int r_index = 0; r_index < BAUM_WELCH_NUM_RUNS; r_index++) {
			vector<double> r_obs;
			vector<int> r_actions;
			for (int i_index = 0; i_index < num_seq_per_run; i_index++) {
				for (int a_index = 0; a_index < seq_length; a_index++) {
					r_obs.push_back(world_truth->get_obs());
					r_actions.push_back(sequence[a_index]);

					world_truth->move(sequence[a_index]);
				}
			}

			obs.push_back(r_obs);
			actions.push_back(r_actions);
		}

		update(world_model,
			   obs,
			   actions);

		cout << "average_standard_deviation:" << endl;
		for (int s_index = 0; s_index < seq_length; s_index++) {
			cout << s_index << ": " << world_model->states[s_index]->average_standard_deviation << endl;
		}

		bool is_success = true;
		for (int s_index = 0; s_index < seq_length; s_index++) {
			if (world_model->states[s_index]->average_standard_deviation > average_standard_deviation * REPETITION_MAX_RATIO) {
				is_success = false;
				break;
			}
		}

		if (is_success) {
			return world_model;
		} else {
			delete world_model;
		}
	}
}
