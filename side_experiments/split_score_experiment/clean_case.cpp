#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "simplest.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_SAMPLES = 4000;

const int EXPLORE_NUM_RANDOM = 20;

const double MIN_MATCH_RATIO = 0.1;

const int SPLIT_NUM_TRIES = 40;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimplest();

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
	for (int t_index = 0; t_index < NUM_SAMPLES; t_index++) {
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
	for (int t_index = 0; t_index < NUM_SAMPLES; t_index++) {
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

	vector<Network*> curr_match_networks;
	vector<Network*> curr_signal_networks;
	double curr_miss_average_guess;

	{
		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)explore_target_vals.size(); h_index++) {
			sum_vals += explore_target_vals[h_index];
		}
		curr_miss_average_guess = sum_vals / (double)explore_target_vals.size();
	}

	double curr_misguess;
	double curr_misguess_standard_deviation;
	eval_signal(explore_vals,
				explore_target_vals,
				curr_match_networks,
				curr_signal_networks,
				curr_miss_average_guess,
				curr_misguess,
				curr_misguess_standard_deviation);

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
				bool is_success = train_score(match_vals,
											  match_target_vals,
											  new_signal_network);
				if (is_success) {
					vector<Network*> potential_match_networks = curr_match_networks;
					potential_match_networks.insert(potential_match_networks.begin(), new_match_network);
					vector<Network*> potential_signal_networks = curr_signal_networks;
					potential_signal_networks.insert(potential_signal_networks.begin(), new_signal_network);
					double potential_miss_average_guess = calc_miss_average_guess(
						explore_vals,
						explore_target_vals,
						potential_match_networks);

					double new_misguess;
					double new_misguess_standard_deviation;
					eval_signal(explore_vals,
								explore_target_vals,
								potential_match_networks,
								potential_signal_networks,
								potential_miss_average_guess,
								new_misguess,
								new_misguess_standard_deviation);

					double misguess_improvement = curr_misguess - new_misguess;
					double min_standard_deviation = min(curr_misguess_standard_deviation, new_misguess_standard_deviation);
					double t_score = misguess_improvement / (min_standard_deviation / sqrt((double)explore_vals.size()));

					cout << "t_score: " << t_score << endl;

					if (t_score >= 1.282) {
						curr_match_networks = potential_match_networks;
						curr_signal_networks = potential_signal_networks;
						curr_miss_average_guess = potential_miss_average_guess;

						curr_misguess = new_misguess;
						curr_misguess_standard_deviation = new_misguess_standard_deviation;

						cout << "new_misguess: " << new_misguess << endl;
						cout << "new_misguess_standard_deviation: " << new_misguess_standard_deviation << endl;
					} else {
						delete new_signal_network;
						delete new_match_network;
					}
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
		double signal = calc_signal(curr_vals,
									curr_match_networks,
									curr_signal_networks,
									curr_miss_average_guess);
		cout << "signal: " << signal << endl;
		problem->print();
		cout << endl;

		delete problem;
	}

	delete problem_type;

	cout << "Done" << endl;
}
