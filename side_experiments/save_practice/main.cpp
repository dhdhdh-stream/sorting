// - what is needed to save a situation?
//   - non-sharp, non-innovative
//     - most generic
//   - can do a full evaluation of location
//   - with location, just greedily head to save

// - if have save, then solutions also don't need to be full?
//   - can take sharp decisions to reach a good spot, then rely on save for the rest

// - have to test if save is anti-synergy with explore

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "network.h"
#include "problem.h"

using namespace std;

default_random_engine generator;

const int MAX_NUM_ACTIONS = 100;

int step_helper(vector<double>& obs,
				Network* actor) {
	vector<double> scores(4);
	for (int a_index = 0; a_index < 4; a_index++) {
		vector<double> inputs = obs;
		inputs.push_back(a_index);
		actor->activate(inputs);
		scores[a_index] = actor->output->acti_vals[0];
	}

	vector<double> expos(4);
	double sum_vals = 0.0;
	for (int v_index = 0; v_index < 4; v_index++) {
		expos[v_index] = exp(scores[v_index]);

		sum_vals += expos[v_index];
	}
	uniform_real_distribution<double> distribution(0.0, sum_vals);
	int rand_val = distribution(generator);
	int action;
	for (int v_index = 0; v_index < 4; v_index++) {
		rand_val -= expos[v_index];
		if (rand_val <= 0.0) {
			action = v_index;
			break;
		}
	}

	return action;
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	// int seed = (unsigned)time(NULL);
	int seed = 1760750341;
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Network* actor = new Network(3);
	Network* critic = new Network(2);

	uniform_int_distribution<int> use_existing_distribution(0, 2);
	geometric_distribution<int> num_random_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	int iter_index = 0;
	while (true) {
		Problem* problem = new Problem();

		vector<int> existing_actions;
		get_existing_solution(existing_actions);

		vector<vector<double>> obs;
		vector<int> actions;
		if (use_existing_distribution(generator) == 0) {
			actions = existing_actions;

			for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
				obs.push_back(problem->get_observations());
				problem->perform_action(actions[a_index]);
			}
		} else {
			uniform_int_distribution<int> num_existing_distribution(0, existing_actions.size()-1);
			int num_existing = num_existing_distribution(generator);
			for (int e_index = 0; e_index < num_existing; e_index++) {
				actions.push_back(existing_actions[e_index]);

				obs.push_back(problem->get_observations());
				problem->perform_action(existing_actions[e_index]);
			}

			int num_random = num_random_distribution(generator);
			for (int r_index = 0; r_index < num_random; r_index++) {
				int random_action = action_distribution(generator);
				actions.push_back(random_action);

				obs.push_back(problem->get_observations());
				problem->perform_action(random_action);
			}

			while (true) {
				vector<double> curr_obs = problem->get_observations();

				int next_action = step_helper(curr_obs,
											  actor);
				actions.push_back(next_action);

				obs.push_back(curr_obs);
				bool is_done = problem->perform_action(next_action);
				if (is_done) {
					break;
				}

				if (obs.size() >= MAX_NUM_ACTIONS) {
					break;
				}
			}
		}

		vector<double> predicted_scores(obs.size());
		{
			critic->activate(obs[obs.size()-1]);
			predicted_scores[obs.size()-1] = critic->output->acti_vals[0];

			double distance;
			if (obs.size() >= MAX_NUM_ACTIONS) {
				distance = MAX_NUM_ACTIONS;
			} else {
				distance = 1.0;
			}
			double error = distance - critic->output->acti_vals[0];
			critic->backprop(error);
		}
		for (int o_index = (int)obs.size()-2; o_index >= 0; o_index--) {
			critic->activate(obs[o_index]);
			predicted_scores[o_index] = critic->output->acti_vals[0];

			double true_distance;
			if (obs.size() >= MAX_NUM_ACTIONS) {
				true_distance = MAX_NUM_ACTIONS;
			} else {
				true_distance = (int)obs.size()-1 + 1 - o_index;
			}
			double predicted_distance = 1.0 + predicted_scores[o_index+1];
			double distance = (true_distance + predicted_distance) / 2.0;
			double error = distance - critic->output->acti_vals[0];
			critic->backprop(error);
		}

		{
			vector<double> actor_inputs = obs[obs.size()-1];
			actor_inputs.push_back(actions[obs.size()-1]);
			actor->activate(actor_inputs);

			double diff = predicted_scores[obs.size()-1] - 0.0;
			double error = diff - actor->output->acti_vals[0];
			actor->backprop(error);
		}
		for (int o_index = (int)obs.size()-2; o_index >= 0; o_index--) {
			vector<double> actor_inputs = obs[o_index];
			actor_inputs.push_back(actions[o_index]);
			actor->activate(actor_inputs);

			double diff = predicted_scores[o_index] - predicted_scores[o_index+1];
			double error = diff - actor->output->acti_vals[0];
			actor->backprop(error);
		}

		delete problem;

		iter_index++;
		if (iter_index % 1000 == 0) {
			cout << iter_index << endl;

			{
				double current_x = 0;
				double current_y = 0;
				vector<double> inputs{current_x, current_y};
				critic->activate(inputs);
				cout << "(" << current_x << "," << current_y << "): " << critic->output->acti_vals[0] << endl;
			}

			{
				double current_x = 1;
				double current_y = 1;
				vector<double> inputs{current_x, current_y};
				critic->activate(inputs);
				cout << "(" << current_x << "," << current_y << "): " << critic->output->acti_vals[0] << endl;
			}

			{
				double current_x = 2;
				double current_y = 2;
				vector<double> inputs{current_x, current_y};
				critic->activate(inputs);
				cout << "(" << current_x << "," << current_y << "): " << critic->output->acti_vals[0] << endl;
			}

			{
				double current_x = 3;
				double current_y = 3;
				vector<double> inputs{current_x, current_y};
				critic->activate(inputs);
				cout << "(" << current_x << "," << current_y << "): " << critic->output->acti_vals[0] << endl;
			}

			{
				double current_x = 1;
				double current_y = 1;
				vector<double> inputs{current_x, current_y, (double)ACTION_UP};
				actor->activate(inputs);
				cout << "ACTION_UP: " << actor->output->acti_vals[0] << endl;
			}

			{
				double current_x = 1;
				double current_y = 1;
				vector<double> inputs{current_x, current_y, (double)ACTION_RIGHT};
				actor->activate(inputs);
				cout << "ACTION_RIGHT: " << actor->output->acti_vals[0] << endl;
			}

			{
				double current_x = 1;
				double current_y = 1;
				vector<double> inputs{current_x, current_y, (double)ACTION_DOWN};
				actor->activate(inputs);
				cout << "ACTION_DOWN: " << actor->output->acti_vals[0] << endl;
			}

			{
				double current_x = 1;
				double current_y = 1;
				vector<double> inputs{current_x, current_y, (double)ACTION_LEFT};
				actor->activate(inputs);
				cout << "ACTION_LEFT: " << actor->output->acti_vals[0] << endl;
			}

			cout << endl;
		}
	}

	delete actor;
	delete critic;

	cout << "Done" << endl;
}
