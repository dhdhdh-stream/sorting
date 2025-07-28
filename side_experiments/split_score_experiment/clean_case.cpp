#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "simpler.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_EXISTING = 1000;
const int NUM_EXPLORE = 4000;

const int EXPLORE_NUM_RANDOM = 20;

const double SEED_RATIO = 0.2;

const int MAX_EPOCHS = 30;
const int ITERS_PER_EPOCH = 10000;
const double MAX_AVERAGE_ERROR = 0.1;

const double MIN_MATCH_RATIO = 0.1;

const int TRAIN_ITERS = 300000;

const int SPLIT_NUM_TRIES = 20;

bool split_helper(vector<vector<double>>& existing_vals,
				  vector<vector<double>>& explore_vals,
				  Network* match_network) {
	vector<int> negative_seeds;
	int num_seeds = SEED_RATIO * (double)explore_vals.size();
	vector<int> initial_possible_indexes(explore_vals.size());
	for (int i_index = 0; i_index < (int)explore_vals.size(); i_index++) {
		initial_possible_indexes[i_index] = i_index;
	}
	for (int s_index = 0; s_index < num_seeds; s_index++) {
		uniform_int_distribution<int> possible_distribution(0, initial_possible_indexes.size()-1);
		int random_index = possible_distribution(generator);
		negative_seeds.push_back(initial_possible_indexes[random_index]);
		initial_possible_indexes.erase(initial_possible_indexes.begin() + random_index);
	}

	uniform_int_distribution<int> is_existing_distribution(0, 1);
	uniform_int_distribution<int> existing_distribution(0, existing_vals.size()-1);
	uniform_int_distribution<int> seed_distribution(0, num_seeds-1);
	int e_index = 0;
	while (true) {
		double sum_errors = 0.0;
		for (int iter_index = 0; iter_index < ITERS_PER_EPOCH; iter_index++) {
			bool is_existing = is_existing_distribution(generator) == 0;

			vector<double> inputs;
			if (is_existing) {
				int random_index = existing_distribution(generator);
				inputs = existing_vals[random_index];
			} else {
				int random_index = seed_distribution(generator);
				int explore_index = negative_seeds[random_index];
				inputs = explore_vals[explore_index];
			}

			match_network->activate(inputs);

			double error;
			if (is_existing) {
				if (match_network->output->acti_vals[0] < 1.0) {
					error = 1.0 - match_network->output->acti_vals[0];
				}
			} else {
				if (match_network->output->acti_vals[0] > -1.0) {
					error = -1.0 - match_network->output->acti_vals[0];
				}
			}

			match_network->backprop(error);

			sum_errors += abs(error);
		}

		double average_error = sum_errors / ITERS_PER_EPOCH;
		if (average_error <= MAX_AVERAGE_ERROR) {
			return true;
		}

		e_index++;
		if (e_index >= MAX_EPOCHS) {
			return false;
		}

		vector<pair<double,int>> explore_acti_vals(explore_vals.size());
		for (int h_index = 0; h_index < (int)explore_vals.size(); h_index++) {
			match_network->activate(explore_vals[h_index]);

			explore_acti_vals[h_index] = {match_network->output->acti_vals[0], h_index};
		}
		sort(explore_acti_vals.begin(), explore_acti_vals.end());
		for (int s_index = 0; s_index < num_seeds; s_index++) {
			negative_seeds[s_index] = explore_acti_vals[s_index].second;
		}
	}

	return true;
}

bool train_score(vector<vector<double>>& vals,
				 vector<double>& target_vals,
				 Network* network,
				 double& highest_signal) {
	uniform_int_distribution<int> distribution(0, vals.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = distribution(generator);
		network->activate(vals[rand_index]);

		double error = target_vals[rand_index] - network->output->acti_vals[0];

		network->backprop(error);
	}

	vector<double> network_vals(vals.size());
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		network->activate(vals[i_index]);
		network_vals[i_index] = network->output->acti_vals[0];
	}

	double sum_target_vals = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		sum_target_vals += target_vals[i_index];
	}
	double average_target_val = sum_target_vals / (double)vals.size();

	double sum_base_misguess = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		sum_base_misguess += (target_vals[i_index] - average_target_val)
			* (target_vals[i_index] - average_target_val);
	}
	double average_base_misguess = sum_base_misguess / (double)vals.size();

	double sum_base_misguess_variance = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		double curr_misguess = (target_vals[i_index] - average_target_val)
			* (target_vals[i_index] - average_target_val);
		sum_base_misguess_variance += (curr_misguess - average_base_misguess) * (curr_misguess - average_base_misguess);
	}
	double base_misguess_standard_deviation = sqrt(sum_base_misguess_variance / (double)vals.size());
	if (base_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		base_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}

	double sum_signal_misguess = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		sum_signal_misguess += (target_vals[i_index] - network_vals[i_index])
			* (target_vals[i_index] - network_vals[i_index]);
	}
	double average_signal_misguess = sum_signal_misguess / (double)vals.size();

	double sum_signal_misguess_variance = 0.0;
	for (int i_index = 0; i_index < (int)vals.size(); i_index++) {
		double curr_misguess = (target_vals[i_index] - network_vals[i_index])
			* (target_vals[i_index] - network_vals[i_index]);
		sum_signal_misguess_variance += (curr_misguess - average_signal_misguess) * (curr_misguess - average_signal_misguess);
	}
	double signal_misguess_standard_deviation = sqrt(sum_signal_misguess_variance / (double)vals.size());
	if (signal_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
		signal_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
	}

	double signal_improvement = average_base_misguess - average_signal_misguess;
	double min_standard_deviation = min(base_misguess_standard_deviation, signal_misguess_standard_deviation);
	double t_score = signal_improvement / (min_standard_deviation / sqrt((double)vals.size()));

	cout << "t_score: " << t_score << endl;

	if (t_score < 2.326) {
		return false;
	}

	highest_signal = network_vals[0];
	for (int i_index = 1; i_index < (int)vals.size(); i_index++) {
		if (network_vals[i_index] > highest_signal) {
			highest_signal = network_vals[i_index];
		}
	}

	cout << "highest_signal: " << highest_signal << endl;

	return true;
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimpler();

	vector<int> start_actions;
	start_actions.push_back(1);
	start_actions.push_back(1);
	start_actions.push_back(1);
	start_actions.push_back(1);

	vector<int> end_actions;
	end_actions.push_back(0);
	end_actions.push_back(0);
	end_actions.push_back(0);
	end_actions.push_back(0);

	/**
	 * - easiest scenario: clean, not mixed
	 */
	vector<vector<double>> existing_vals;
	vector<double> existing_target_vals;
	for (int t_index = 0; t_index < NUM_EXISTING; t_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_vals;

		curr_vals.push_back(problem->get_observations()[0]);
		for (int a_index = 0; a_index < (int)start_actions.size(); a_index++) {
			problem->perform_action(start_actions[a_index]);

			curr_vals.push_back(problem->get_observations()[0]);
		}

		curr_vals.push_back(problem->get_observations()[0]);
		for (int a_index = 0; a_index < (int)end_actions.size(); a_index++) {
			problem->perform_action(end_actions[a_index]);

			curr_vals.push_back(problem->get_observations()[0]);
		}

		existing_vals.push_back(curr_vals);

		double target_val = problem->score_result();
		existing_target_vals.push_back(target_val);

		delete problem;
	}

	vector<vector<double>> explore_vals;
	vector<double> explore_target_vals;
	uniform_int_distribution<int> random_action_distribution(0, problem_type->num_possible_actions()-1);
	for (int t_index = 0; t_index < NUM_EXPLORE; t_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_vals;

		curr_vals.push_back(problem->get_observations()[0]);
		for (int a_index = 0; a_index < (int)start_actions.size(); a_index++) {
			problem->perform_action(start_actions[a_index]);

			curr_vals.push_back(problem->get_observations()[0]);
		}

		for (int a_index = 0; a_index < EXPLORE_NUM_RANDOM; a_index++) {
			problem->perform_action(random_action_distribution(generator));
		}

		curr_vals.push_back(problem->get_observations()[0]);
		for (int a_index = 0; a_index < (int)end_actions.size(); a_index++) {
			problem->perform_action(end_actions[a_index]);

			curr_vals.push_back(problem->get_observations()[0]);
		}

		explore_vals.push_back(curr_vals);

		double target_val = problem->score_result();
		explore_target_vals.push_back(target_val);

		delete problem;
	}

	double best_highest_signal = numeric_limits<double>::lowest();
	Network* best_match_network = NULL;
	Network* best_signal_network = NULL;
	int num_min_match = MIN_MATCH_RATIO * (int)explore_vals.size();
	for (int t_index = 0; t_index < SPLIT_NUM_TRIES; t_index++) {
		Network* new_match_network = new Network(10);
		bool split_is_success = split_helper(existing_vals,
											 explore_vals,
											 new_match_network);
		if (split_is_success) {
			vector<vector<double>> match_vals;
			vector<double> match_target_vals;
			for (int h_index = 0; h_index < (int)explore_vals.size(); h_index++) {
				new_match_network->activate(explore_vals[h_index]);
				if (new_match_network->output->acti_vals[0] > 0.0) {
					match_vals.push_back(explore_vals[h_index]);
					match_target_vals.push_back(explore_target_vals[h_index]);
				}
			}

			if ((int)match_vals.size() >= num_min_match) {
				Network* new_signal_network = new Network(10);
				double new_highest_signal;
				bool is_success = train_score(match_vals,
											  match_target_vals,
											  new_signal_network,
											  new_highest_signal);
				if (is_success && new_highest_signal > best_highest_signal) {
					best_highest_signal = new_highest_signal;

					if (best_match_network != NULL) {
						delete best_match_network;
					}
					best_match_network = new_match_network;

					if (best_signal_network != NULL) {
						delete best_signal_network;
					}
					best_signal_network = new_signal_network;
				} else {
					delete new_signal_network;
					delete new_match_network;
				}
			} else {
				cout << "min match fail" << endl;
				delete new_match_network;
			}
		} else {
			cout << "split fail" << endl;
			delete new_match_network;
		}
	}

	for (int t_index = 0; t_index < 100; t_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> curr_vals;

		curr_vals.push_back(problem->get_observations()[0]);
		for (int a_index = 0; a_index < (int)start_actions.size(); a_index++) {
			problem->perform_action(start_actions[a_index]);

			curr_vals.push_back(problem->get_observations()[0]);
		}

		for (int a_index = 0; a_index < EXPLORE_NUM_RANDOM; a_index++) {
			problem->perform_action(random_action_distribution(generator));
		}

		curr_vals.push_back(problem->get_observations()[0]);
		for (int a_index = 0; a_index < (int)end_actions.size(); a_index++) {
			problem->perform_action(end_actions[a_index]);

			curr_vals.push_back(problem->get_observations()[0]);
		}

		cout << t_index << endl;
		best_match_network->activate(curr_vals);
		cout << "best_match_network->output->acti_vals[0]: " << best_match_network->output->acti_vals[0] << endl;
		best_signal_network->activate(curr_vals);
		cout << "best_signal_network->output->acti_vals[0]: " << best_signal_network->output->acti_vals[0] << endl;
		problem->print();
		cout << endl;

		delete problem;
	}

	delete problem_type;

	cout << "Done" << endl;
}
