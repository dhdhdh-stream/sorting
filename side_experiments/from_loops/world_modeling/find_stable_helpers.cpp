#include "find_stable_helpers.h"

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

	geometric_distribution<int> seq_length_distribution(0.3);
	uniform_real_distribution<double> distribution(0.0, 1.0);
	while (true) {
		int seq_length = 1 + seq_length_distribution(generator);
		vector<int> sequence;
		for (int a_index = 0; a_index < seq_length; a_index++) {
			sequence.push_back(random_action_distribution(generator));
		}

		WorldModel* world_model = new WorldModel();
		for (int s_index = 0; s_index < seq_length; s_index++) {
			WorldState* world_state = new WorldState();
			world_state->id = s_index;
			world_model->states.push_back(world_state);

			world_state->average_val = average_val;
			world_state->average_standard_deviation = average_standard_deviation;

			world_state->transitions = vector<vector<double>>(NUM_ACTIONS);
			for (int a_index = 0; a_index < NUM_ACTIONS; a_index++) {
				vector<double> a_transition(seq_length);
				double sum_vals = 0.0;
				for (int end_index = 0; end_index < seq_length; end_index++) {
					a_transition[end_index] = distribution(generator);

					sum_vals += a_transition[end_index];
				}
				for (int end_index = 0; end_index < seq_length; end_index++) {
					a_transition[end_index] /= sum_vals;
				}

				world_state->transitions[a_index] = a_transition;
			}
		}
		{
			world_model->starting_likelihood = vector<double>(seq_length);
			double sum_vals = 0.0;
			for (int s_index = 0; s_index < seq_length; s_index++) {
				world_model->starting_likelihood[s_index] = distribution(generator);
			}
			for (int s_index = 0; s_index < seq_length; s_index++) {
				world_model->starting_likelihood[s_index] /= sum_vals;
			}
		}

		for (int i_index = 0; i_index < INITIAL_SEQ_ITERATIONS; i_index++) {
			for (int a_index = 0; a_index < seq_length; a_index++) {
				world_truth->move(sequence[a_index]);
			}
		}

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
		}

		update(world_model,
			   obs,
			   actions);

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
