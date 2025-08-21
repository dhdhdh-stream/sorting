#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "helpers.h"
#include "network.h"
#include "simpler.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_SAMPLES = 4000;

const int EXPLORE_NUM_RANDOM = 20;

const int NUM_TRIES = 20;

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

	uniform_int_distribution<int> random_action_distribution(0, problem_type->num_possible_actions()-1);

	vector<vector<double>> explore_vals;
	vector<double> explore_target_vals;
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

	vector<double> predicted_vals(explore_vals.size(), curr_miss_average_guess);

	for (int t_index = 0; t_index < NUM_TRIES; t_index++) {
		Network* new_match_network = new Network(10);
		Network* new_score_network = new Network(10);
		bool is_success = alternate_train_helper(explore_vals,
												 explore_target_vals,
												 predicted_vals,
												 new_match_network,
												 new_score_network);
		if (is_success) {
			vector<Network*> potential_match_networks = curr_match_networks;
			potential_match_networks.insert(potential_match_networks.begin(), new_match_network);
			vector<Network*> potential_signal_networks = curr_signal_networks;
			potential_signal_networks.insert(potential_signal_networks.begin(), new_score_network);
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

				for (int h_index = 0; h_index < (int)explore_vals.size(); h_index++) {
					predicted_vals[h_index] = calc_signal(explore_vals[h_index],
														  curr_match_networks,
														  curr_signal_networks,
														  curr_miss_average_guess);
				}
			} else {
				delete new_match_network;
				delete new_score_network;
			}
		} else {
			delete new_match_network;
			delete new_score_network;
		}
	}

	cout << "Done" << endl;
}
