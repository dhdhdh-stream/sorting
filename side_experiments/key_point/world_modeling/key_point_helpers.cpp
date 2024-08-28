#include "key_point_helpers.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "key_point.h"
#include "world_truth.h"

using namespace std;

const int KEY_POINT_SEQUENCE_LENGTH = 5;
/**
 * - length should be long enough to handle any delay
 */

const int NUM_CANDIDATES = 40;

const int UNKNOWN_ACTIONS = 10;

const int NUM_PATHS_TO_GATHER = 10;
const int MAX_PATH_LENGTH = 30;
const int GATHER_PATHS_MAX_TRIES = 500;

const int PATH_REPEAT_SUCCESSES_NEEDED = 5;
const int CONSECUTIVE_REPEAT_FAILURES_ALLOWED = 20;

const int VERIFY_ITERS_PER_START = 1000;

KeyPoint* create_potential_key_point(WorldTruth* world_truth) {
	vector<int> actions(KEY_POINT_SEQUENCE_LENGTH);
	uniform_int_distribution<int> action_distribution(0, NUM_ACTIONS-1);
	for (int a_index = 0; a_index < KEY_POINT_SEQUENCE_LENGTH; a_index++) {
		actions[a_index] = action_distribution(generator);
	}

	vector<double> best_obs;
	double best_contrast = 0.0;
	for (int c_index = 0; c_index < NUM_CANDIDATES; c_index++) {
		for (int a_index = 0; a_index < UNKNOWN_ACTIONS; a_index++) {
			world_truth->move(action_distribution(generator));
		}

		vector<double> curr_obs;
		curr_obs.push_back(world_truth->get_obs());
		for (int a_index = 0; a_index < KEY_POINT_SEQUENCE_LENGTH; a_index++) {
			world_truth->move(actions[a_index]);

			curr_obs.push_back(world_truth->get_obs());
		}

		double curr_contrast = 0.0;
		for (int o_index = 0; o_index < (int)curr_obs.size()-1; o_index++) {
			curr_contrast += abs(curr_obs[o_index] - curr_obs[o_index+1]);
		}

		if (curr_contrast > best_contrast) {
			best_obs = curr_obs;
			best_contrast = curr_contrast;
		}
	}

	return new KeyPoint(best_obs,
						actions);
}

bool find_paths_for_potential_verify_helpers(
		WorldTruth* world_truth,
		KeyPoint* potential,
		vector<int>& actions) {
	uniform_int_distribution<int> action_distribution(0, NUM_ACTIONS-1);
	uniform_int_distribution<int> unknown_distribution(0, 9);
	for (int i_index = 0; i_index < PATH_REPEAT_SUCCESSES_NEEDED; i_index++) {
		int fail_count = 0;
		while (true) {
			while (true) {
				int num_initial_unknown = unknown_distribution(generator);
				for (int a_index = 0; a_index < num_initial_unknown; a_index++) {
					world_truth->move(action_distribution(generator));
				}

				bool is_match = potential->match(world_truth);

				if (is_match) {
					break;
				}
			}

			for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
				world_truth->move(actions[a_index]);
			}

			bool setup_is_match = potential->match(world_truth);
			if (setup_is_match) {
				break;
			} else {
				fail_count++;
				if (fail_count > CONSECUTIVE_REPEAT_FAILURES_ALLOWED) {
					return false;
				}
			}
		}
	}

	return true;
}

bool find_paths_for_potential(WorldTruth* world_truth,
							  KeyPoint* potential,
							  vector<vector<double>>& path_obs,
							  vector<vector<int>>& path_actions) {
	uniform_int_distribution<int> action_distribution(0, NUM_ACTIONS-1);
	uniform_int_distribution<int> unknown_distribution(0, 9);
	for (int t_index = 0; t_index < GATHER_PATHS_MAX_TRIES; t_index++) {
		while (true) {
			int num_initial_unknown = unknown_distribution(generator);
			for (int a_index = 0; a_index < num_initial_unknown; a_index++) {
				world_truth->move(action_distribution(generator));
			}

			bool is_match = potential->match(world_truth);

			if (is_match) {
				break;
			}
		}

		vector<double> unknown_obs;
		vector<int> unknown_actions;

		while (true) {
			/**
			 * TODO: remove duplicates
			 */

			int num_explore_unknown = unknown_distribution(generator);
			for (int a_index = 0; a_index < num_explore_unknown; a_index++) {
				unknown_obs.push_back(world_truth->get_obs());

				int action = action_distribution(generator);
				world_truth->move(action);
				unknown_actions.push_back(action);
			}

			bool is_match = potential->match(world_truth,
											 unknown_actions,
											 unknown_obs);

			if (is_match) {
				bool verify_is_success = find_paths_for_potential_verify_helpers(
					world_truth,
					potential,
					unknown_actions);

				if (verify_is_success) {
					path_obs.push_back(unknown_obs);
					path_actions.push_back(unknown_actions);
				}

				break;
			} else if ((int)unknown_actions.size() > MAX_PATH_LENGTH) {
				break;
			}
		}

		if ((int)path_obs.size() >= NUM_PATHS_TO_GATHER) {
			return true;
		}
	}

	return false;
}

/**
 * - if unique, then path success likelihood should be constant
 *   - if not and counter path taken, success likelihood should increase
 *     - as edge case will be removed
 */
bool verify_potential_uniqueness(WorldTruth* world_truth,
								 KeyPoint* potential,
								 vector<vector<int>>& path_actions) {
	uniform_int_distribution<int> action_distribution(0, NUM_ACTIONS-1);
	uniform_int_distribution<int> unknown_distribution(0, 9);
	for (int path_index = 0; path_index < (int)path_actions.size(); path_index++) {
		vector<int> success_counts((int)path_actions.size(), 0);
		for (int p_index = 0; p_index < (int)path_actions.size(); p_index++) {
			int count = 0;

			while (true) {
				while (true) {
					int num_initial_unknown = unknown_distribution(generator);
					for (int a_index = 0; a_index < num_initial_unknown; a_index++) {
						world_truth->move(action_distribution(generator));
					}

					bool is_match = potential->match(world_truth);

					if (is_match) {
						break;
					}
				}

				for (int a_index = 0; a_index < (int)path_actions[p_index].size(); a_index++) {
					world_truth->move(path_actions[p_index][a_index]);
				}

				bool setup_is_match = potential->match(world_truth);
				if (!setup_is_match) {
					continue;
				}

				for (int a_index = 0; a_index < (int)path_actions[path_index].size(); a_index++) {
					world_truth->move(path_actions[path_index][a_index]);
				}

				bool eval_is_match = potential->match(world_truth);
				if (eval_is_match) {
					success_counts[p_index]++;
				}

				count++;
				if (count >= VERIFY_ITERS_PER_START) {
					break;
				}
			}
		}

		int sum_counts = 0;
		for (int p_index = 0; p_index < (int)path_actions.size(); p_index++) {
			sum_counts += success_counts[p_index];
		}
		double success_ratio = (double)sum_counts / (double)path_actions.size() / (double)VERIFY_ITERS_PER_START;

		double standard_deviation = sqrt(success_ratio * (1.0 - success_ratio));

		for (int p_index = 0; p_index < (int)path_actions.size(); p_index++) {
			double curr_ratio = (double)success_counts[p_index] / (double)VERIFY_ITERS_PER_START;
			double t_score = abs(success_ratio - curr_ratio) / (standard_deviation / sqrt(VERIFY_ITERS_PER_START));
			if (t_score > 3.09) {
				cout << "t_score: " << t_score << endl;
				return false;
			}
		}

		cout << path_index << " success" << endl;
	}

	return true;
}
