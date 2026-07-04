// - extra samples definitely helps
//   - though if only training for specific paths, might be OK

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

const int NUM_STATES = 4;
// const int NUM_STATES = 2;
// const int NUM_STATES = 1;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<double> context_init_means(NUM_STATES+1, 0.0);
	vector<double> context_init_deviations(NUM_STATES+1, 1.0);
	Network* context_network = new Network(NUM_STATES+1, context_init_means, context_init_deviations, NUM_STATES);
	double context_hidden_1_average_max_update = 0.0;
	double context_hidden_2_average_max_update = 0.0;
	double context_hidden_3_average_max_update = 0.0;
	double context_output_average_max_update = 0.0;
	vector<double> action_init_means(NUM_STATES+4, 0.0);
	vector<double> action_init_deviations(NUM_STATES+4, 1.0);
	Network* action_network = new Network(NUM_STATES+4, action_init_means, action_init_deviations, NUM_STATES);
	double action_hidden_1_average_max_update = 0.0;
	double action_hidden_2_average_max_update = 0.0;
	double action_hidden_3_average_max_update = 0.0;
	double action_output_average_max_update = 0.0;
	vector<double> final_init_means(NUM_STATES, 0.0);
	vector<double> final_init_deviations(NUM_STATES, 1.0);
	Network* final_network = new Network(NUM_STATES, final_init_means, final_init_deviations, 1);
	double final_hidden_1_average_max_update = 0.0;
	double final_hidden_2_average_max_update = 0.0;
	double final_hidden_3_average_max_update = 0.0;
	double final_output_average_max_update = 0.0;

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	uniform_int_distribution<int> context_distribution(0, 4);

	// for (int iter_index = 0; iter_index < 300000; iter_index++) {
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

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
		double target_val;
		if (sum_distance == 1) {
			target_val = 1.0;
		} else {
			target_val = 0.0;
		}

		vector<NetworkHistory*> context_network_histories;
		vector<NetworkHistory*> action_network_histories;

		vector<double> state(NUM_STATES, 0.0);
		for (int a_index = 0; a_index < (int)curr_contexts.size(); a_index++) {
			{
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				inputs.push_back(curr_contexts[a_index]);
				NetworkHistory* network_history = new NetworkHistory();
				context_network->activate(inputs,
										  network_history);
				context_network_histories.push_back(network_history);
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					state[s_index] += context_network->output->acti_vals[s_index];
				}
			}

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == curr_actions[a_index]) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				NetworkHistory* network_history = new NetworkHistory();
				action_network->activate(inputs,
										 network_history);
				action_network_histories.push_back(network_history);
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					state[s_index] += action_network->output->acti_vals[s_index];
				}
			}
		}

		final_network->activate(state);

		vector<double> final_errors{target_val - final_network->output->acti_vals[0]};
		final_network->backprop(final_errors);

		vector<double> state_errors(NUM_STATES, 0.0);
		for (int i_index = 0; i_index < NUM_STATES; i_index++) {
			state_errors[i_index] += final_network->raw_input->errors[i_index];
			final_network->raw_input->errors[i_index] = 0.0;
		}

		for (int h_index = (int)curr_contexts.size()-1; h_index >= 0; h_index--) {
			action_network->backprop(state_errors,
									 action_network_histories[h_index]);
			delete action_network_histories[h_index];
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				state_errors[s_index] += action_network->raw_input->errors[s_index];
				action_network->raw_input->errors[s_index] = 0.0;
			}

			context_network->backprop(state_errors,
									  context_network_histories[h_index]);
			delete context_network_histories[h_index];
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				state_errors[s_index] += context_network->raw_input->errors[s_index];
				context_network->raw_input->errors[s_index] = 0.0;
			}
		}

		if (iter_index % 20 == 0) {
			context_network->init_update(context_hidden_1_average_max_update,
										 context_hidden_2_average_max_update,
										 context_hidden_3_average_max_update,
										 context_output_average_max_update);
			action_network->init_update(action_hidden_1_average_max_update,
										action_hidden_2_average_max_update,
										action_hidden_3_average_max_update,
										action_output_average_max_update);
			final_network->init_update(final_hidden_1_average_max_update,
									   final_hidden_2_average_max_update,
									   final_hidden_3_average_max_update,
									   final_output_average_max_update);
		}
	}

	// temp
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

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
		double target_val;
		if (sum_distance == 1) {
			target_val = 1.0;
		} else {
			target_val = 0.0;
		}

		vector<double> state(NUM_STATES, 0.0);
		for (int a_index = 0; a_index < (int)curr_contexts.size(); a_index++) {
			{
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				inputs.push_back(curr_contexts[a_index]);
				context_network->activate(inputs);
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					state[s_index] += context_network->output->acti_vals[s_index];
				}
			}

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), state.begin(), state.end());
				for (int i_index = 0; i_index < NUM_STATES; i_index++) {
					if (i_index == curr_actions[a_index]) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				action_network->activate(inputs);
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					state[s_index] += action_network->output->acti_vals[s_index];
				}
			}
		}

		final_network->activate(state);

		cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
		cout << "target_val: " << target_val << endl;

		cout << endl;
	}
	{
		double sum_misguess = 0.0;
		for (int iter_index = 0; iter_index < 4000; iter_index++) {
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
			double target_val;
			if (sum_distance == 1) {
				target_val = 1.0;
			} else {
				target_val = 0.0;
			}

			vector<double> state(NUM_STATES, 0.0);
			for (int a_index = 0; a_index < (int)curr_contexts.size(); a_index++) {
				{
					vector<double> inputs;
					inputs.insert(inputs.end(), state.begin(), state.end());
					inputs.push_back(curr_contexts[a_index]);
					context_network->activate(inputs);
					for (int s_index = 0; s_index < NUM_STATES; s_index++) {
						state[s_index] += context_network->output->acti_vals[s_index];
					}
				}

				{
				vector<double> inputs;
					inputs.insert(inputs.end(), state.begin(), state.end());
					for (int i_index = 0; i_index < NUM_STATES; i_index++) {
						if (i_index == curr_actions[a_index]) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}
					action_network->activate(inputs);
					for (int s_index = 0; s_index < NUM_STATES; s_index++) {
						state[s_index] += action_network->output->acti_vals[s_index];
					}
				}
			}

			final_network->activate(state);

			sum_misguess += (target_val - final_network->output->acti_vals[0])
				* (target_val - final_network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / 4000.0;
		cout << "misguess_average: " << misguess_average << endl;
	}

	cout << "Done" << endl;
}
