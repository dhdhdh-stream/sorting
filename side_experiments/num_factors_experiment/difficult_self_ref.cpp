#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "network.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_OBS = 5;
const int SEQUENCE_LENGTH = 10;

const int NUM_SAMPLES = 10000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<int>> contexts;
	vector<vector<int>> actions;
	vector<double> target_vals;

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	uniform_int_distribution<int> context_distribution(0, 4);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = num_actions_distribution(generator);
		
		vector<int> curr_contexts;
		vector<int> curr_actions;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			if (context_distribution(generator) == 0) {
				curr_contexts.push_back(1);
			} else {
				curr_contexts.push_back(0);
			}
			curr_actions.push_back(action_distribution(generator));
		}
		contexts.push_back(curr_contexts);
		actions.push_back(curr_actions);

		int sum_distance = 0;
		for (int a_index = 0; a_index < (int)curr_contexts.size(); a_index++) {
			switch (curr_actions[a_index]) {
			case 0:
				if (curr_contexts[a_index] == 1) {
					sum_distance++;
				} else {
					sum_distance--;
				}
				break;
			case 1:
				if (curr_contexts[a_index] == 1) {
					sum_distance--;
				} else {
					sum_distance++;
				}
				break;
			}
		}
		if (sum_distance == 1) {
			target_vals.push_back(1.0);
		} else {
			target_vals.push_back(0.0);
		}
	}

	Network* context_network = new Network(LEAKY_LAYER, 5, 4);
	Network* action_network = new Network(LEAKY_LAYER, 9, 4);
	Network* final_network = new Network(LEAKY_LAYER, 4, 1);

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);

	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<NetworkHistory*> context_network_histories;
		vector<NetworkHistory*> action_network_histories;

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)contexts[sequence_index].size(); a_index++) {
			{
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				inputs.push_back(contexts[sequence_index][a_index]);
				NetworkHistory* network_history = new NetworkHistory();
				context_network->activate(inputs,
										  network_history);
				context_network_histories.push_back(network_history);
				for (int s_index = 0; s_index < 4; s_index++) {
					state[s_index] += context_network->output->acti_vals[s_index];
				}
			}

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == actions[sequence_index][a_index]) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				NetworkHistory* network_history = new NetworkHistory();
				action_network->activate(inputs,
										 network_history);
				action_network_histories.push_back(network_history);
				for (int s_index = 0; s_index < 4; s_index++) {
					state[s_index] += action_network->output->acti_vals[s_index];
				}
			}
		}

		final_network->activate(state);

		vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
		final_network->backprop(final_errors);

		vector<double> state_errors(4, 0.0);
		for (int i_index = 0; i_index < 4; i_index++) {
			state_errors[i_index] += final_network->input->errors[i_index];
			final_network->input->errors[i_index] = 0.0;
		}

		for (int h_index = (int)contexts[sequence_index].size()-1; h_index >= 0; h_index--) {
			action_network->backprop(state_errors,
									 action_network_histories[h_index]);
			delete action_network_histories[h_index];
			for (int s_index = 0; s_index < 4; s_index++) {
				state_errors[s_index] += action_network->input->errors[s_index];
				action_network->input->errors[s_index] = 0.0;
			}

			context_network->backprop(state_errors,
									  context_network_histories[h_index]);
			delete context_network_histories[h_index];
			for (int s_index = 0; s_index < 4; s_index++) {
				state_errors[s_index] += context_network->input->errors[s_index];
				context_network->input->errors[s_index] = 0.0;
			}
		}

		if (iter_index % 20 == 0) {
			context_network->update();
			action_network->update();
			final_network->update();
		}
	}

	// temp
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int sequence_index = sequence_distribution(generator);

		vector<double> state(4, 0.0);
		for (int a_index = 0; a_index < (int)contexts[sequence_index].size(); a_index++) {
			{
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				inputs.push_back(contexts[sequence_index][a_index]);
				context_network->activate(inputs);
				for (int s_index = 0; s_index < 4; s_index++) {
					state[s_index] += context_network->output->acti_vals[s_index];
				}
			}

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == actions[sequence_index][a_index]) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				action_network->activate(inputs);
				for (int s_index = 0; s_index < 4; s_index++) {
					state[s_index] += action_network->output->acti_vals[s_index];
				}
			}
		}

		final_network->activate(state);

		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
		cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;

		cout << endl;
	}
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state(4, 0.0);
			for (int a_index = 0; a_index < (int)contexts[h_index].size(); a_index++) {
				{
					vector<double> inputs;
					inputs.insert(inputs.end(), state.begin(), state.end());
					inputs.push_back(contexts[h_index][a_index]);
					context_network->activate(inputs);
					for (int s_index = 0; s_index < 4; s_index++) {
						state[s_index] += context_network->output->acti_vals[s_index];
					}
				}

				{
				vector<double> inputs;
					inputs.insert(inputs.end(), state.begin(), state.end());
					for (int i_index = 0; i_index < 4; i_index++) {
						if (i_index == actions[h_index][a_index]) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}
					action_network->activate(inputs);
					for (int s_index = 0; s_index < 4; s_index++) {
						state[s_index] += action_network->output->acti_vals[s_index];
					}
				}
			}

			final_network->activate(state);

			sum_misguess += (target_vals[h_index] - final_network->output->acti_vals[0])
				* (target_vals[h_index] - final_network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	cout << "Done" << endl;
}
