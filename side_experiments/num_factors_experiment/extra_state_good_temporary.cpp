// - actually does work

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

const int NUM_SAMPLES = 10000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<vector<int>> sequences;
	vector<vector<int>> contexts;
	vector<double> target_vals;

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	// uniform_int_distribution<int> context_distribution(0, 9);
	uniform_int_distribution<int> context_distribution(0, 4);
	for (int i_index = 0; i_index < NUM_SAMPLES; i_index++) {
		int num_actions = 1 + num_actions_distribution(generator);
		
		vector<int> curr_sequence;
		vector<int> curr_context;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			curr_sequence.push_back(action_distribution(generator));
			if (context_distribution(generator) == 0) {
				curr_context.push_back(1);
			} else {
				curr_context.push_back(0);
			}
		}
		sequences.push_back(curr_sequence);
		contexts.push_back(curr_context);

		int sum_distance = 0;
		for (int a_index = 0; a_index < (int)curr_sequence.size(); a_index++) {
			switch (curr_sequence[a_index]) {
			case 0:
				if (curr_context[a_index] == 1) {
					sum_distance--;
				} else {
					sum_distance++;
				}
				break;
			case 1:
				if (curr_context[a_index] == 1) {
					sum_distance++;
				} else {
					sum_distance--;
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

	uniform_int_distribution<int> sequence_distribution(0, NUM_SAMPLES-1);

	Network* context_one_network = new Network(LEAKY_LAYER, 5, 4);
	Network* context_two_network = new Network(LEAKY_LAYER, 2, 1);
	Network* network = new Network(LEAKY_LAYER, 4, 1);

	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(4, 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];
			state_one[action] += 1.0;
		}

		network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
		network->backprop(final_errors);

		if (iter_index % 20 == 0) {
			network->update();
		}
	}
	for (int i_index = 0; i_index < (int)network->input->errors.size(); i_index++) {
		network->input->errors[i_index] = 0.0;
	}
	// for (int iter_index = 0; iter_index < 200000; iter_index++) {
	for (int iter_index = 0; iter_index < 500000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(4, 0.0);

		vector<NetworkHistory*> context_one_network_histories;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			vector<double> inputs;
			inputs.push_back(contexts[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				if (action == s_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			NetworkHistory* history = new NetworkHistory();
			context_one_network->activate(inputs,
										  history);
			context_one_network_histories.push_back(history);
			for (int o_index = 0; o_index < 4; o_index++) {
				state_one[o_index] += context_one_network->output->acti_vals[o_index];
			}

			double ratio = 1.0 - iter_index / 500000.0;
			state_one[action] += ratio;
		}

		network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
		network->backprop(final_errors);

		vector<double> error_one(4, 0.0);
		for (int i_index = 0; i_index < 4; i_index++) {
			error_one[i_index] += network->input->errors[i_index]
				/ (double)sequences[sequence_index].size();
			// error_one[i_index] += network->input->errors[i_index];
			network->input->errors[i_index] = 0.0;
		}

		for (int h_index = (int)context_one_network_histories.size()-1; h_index >= 0; h_index--) {
			context_one_network->backprop(error_one,
										  context_one_network_histories[h_index]);
			delete context_one_network_histories[h_index];
		}

		if (iter_index % 20 == 0) {
			context_one_network->update();
			network->update();
		}
	}
	// for (int iter_index = 0; iter_index < 100000; iter_index++) {
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(4, 0.0);

		vector<NetworkHistory*> context_one_network_histories;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			vector<double> inputs;
			inputs.push_back(contexts[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				if (action == s_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			NetworkHistory* history = new NetworkHistory();
			context_one_network->activate(inputs,
										  history);
			context_one_network_histories.push_back(history);
			for (int o_index = 0; o_index < 4; o_index++) {
				state_one[o_index] += context_one_network->output->acti_vals[o_index];
			}
		}

		network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
		network->backprop(final_errors);

		vector<double> error_one(4, 0.0);
		for (int i_index = 0; i_index < 4; i_index++) {
			error_one[i_index] += network->input->errors[i_index]
				/ (double)sequences[sequence_index].size();
			// error_one[i_index] += network->input->errors[i_index];
			network->input->errors[i_index] = 0.0;
		}

		for (int h_index = (int)context_one_network_histories.size()-1; h_index >= 0; h_index--) {
			context_one_network->backprop(error_one,
										  context_one_network_histories[h_index]);
			delete context_one_network_histories[h_index];
		}

		if (iter_index % 20 == 0) {
			context_one_network->update();
			network->update();
		}
	}

	// temp
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(4, 0.0);

		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			// cout << "a_index: " << a_index << endl;

			int action = sequences[sequence_index][a_index];
			// cout << "action: " << action << endl;

			// cout << "contexts[sequence_index][a_index]: " << contexts[sequence_index][a_index] << endl;

			vector<double> inputs;
			inputs.push_back(contexts[sequence_index][a_index]);
			for (int s_index = 0; s_index < 4; s_index++) {
				if (action == s_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}
			context_one_network->activate(inputs);
			for (int o_index = 0; o_index < 4; o_index++) {
				state_one[o_index] += context_one_network->output->acti_vals[o_index];
				// cout << o_index << ": " << context_one_network->output->acti_vals[o_index] << endl;
			}
		}

		network->activate(state_one);

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;
	}
	// temp
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state_one(4, 0.0);

			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				int action = sequences[h_index][a_index];

				vector<double> inputs;
				inputs.push_back(contexts[h_index][a_index]);
				for (int s_index = 0; s_index < 4; s_index++) {
					if (action == s_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				context_one_network->activate(inputs);
				for (int o_index = 0; o_index < 4; o_index++) {
					state_one[o_index] += context_one_network->output->acti_vals[o_index];
				}
			}

			network->activate(state_one);

			sum_misguess += (target_vals[h_index] - network->output->acti_vals[0])
				* (target_vals[h_index] - network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	for (int i_index = 0; i_index < (int)context_one_network->input->errors.size(); i_index++) {
		context_one_network->input->errors[i_index] = 0.0;
	}
	for (int iter_index = 0; iter_index < 500000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(4, 0.0);
		double state_two = 0.0;

		double ratio = 1.0 - iter_index / 500000.0;

		vector<NetworkHistory*> context_one_network_histories;
		vector<NetworkHistory*> context_two_network_histories;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			{
				vector<double> inputs;
				inputs.push_back(state_two);
				inputs.push_back(contexts[sequence_index][a_index]);
				NetworkHistory* history = new NetworkHistory();
				context_two_network->activate(inputs,
											  history);
				context_two_network_histories.push_back(history);
				state_two += context_two_network->output->acti_vals[0];
			}

			{
				vector<double> inputs;
				inputs.push_back(ratio * contexts[sequence_index][a_index] + state_two);
				for (int s_index = 0; s_index < 4; s_index++) {
					if (action == s_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				NetworkHistory* history = new NetworkHistory();
				context_one_network->activate(inputs,
											  history);
				context_one_network_histories.push_back(history);
				for (int o_index = 0; o_index < 4; o_index++) {
					state_one[o_index] += context_one_network->output->acti_vals[o_index];
				}
			}
		}

		network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
		network->backprop(final_errors);

		vector<double> error_one(4, 0.0);
		for (int i_index = 0; i_index < 4; i_index++) {
			error_one[i_index] += network->input->errors[i_index]
				/ (double)sequences[sequence_index].size();
			// error_one[i_index] += network->input->errors[i_index];
			network->input->errors[i_index] = 0.0;
		}
		double error_two = 0.0;

		for (int h_index = (int)context_one_network_histories.size()-1; h_index >= 0; h_index--) {
			context_one_network->backprop(error_one,
										  context_one_network_histories[h_index]);
			delete context_one_network_histories[h_index];
			error_two += context_one_network->input->errors[0];
			context_one_network->input->errors[0] = 0.0;

			vector<double> errors{error_two};
			context_two_network->backprop(errors,
										  context_two_network_histories[h_index]);
			delete context_two_network_histories[h_index];
			error_two += context_two_network->input->errors[0];
			context_two_network->input->errors[0] = 0.0;
		}

		if (iter_index % 20 == 0) {
			context_one_network->update();
			context_two_network->update();
			network->update();
		}
	}
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(4, 0.0);
		double state_two = 0.0;

		vector<NetworkHistory*> context_one_network_histories;
		vector<NetworkHistory*> context_two_network_histories;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			{
				vector<double> inputs;
				inputs.push_back(state_two);
				inputs.push_back(contexts[sequence_index][a_index]);
				NetworkHistory* history = new NetworkHistory();
				context_two_network->activate(inputs,
											  history);
				context_two_network_histories.push_back(history);
				state_two += context_two_network->output->acti_vals[0];
			}

			{
				vector<double> inputs;
				inputs.push_back(state_two);
				for (int s_index = 0; s_index < 4; s_index++) {
					if (action == s_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				NetworkHistory* history = new NetworkHistory();
				context_one_network->activate(inputs,
											  history);
				context_one_network_histories.push_back(history);
				for (int o_index = 0; o_index < 4; o_index++) {
					state_one[o_index] += context_one_network->output->acti_vals[o_index];
				}
			}
		}

		network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - network->output->acti_vals[0]};
		network->backprop(final_errors);

		vector<double> error_one(4, 0.0);
		for (int i_index = 0; i_index < 4; i_index++) {
			error_one[i_index] += network->input->errors[i_index]
				/ (double)sequences[sequence_index].size();
			// error_one[i_index] += network->input->errors[i_index];
			network->input->errors[i_index] = 0.0;
		}
		double error_two = 0.0;

		for (int h_index = (int)context_one_network_histories.size()-1; h_index >= 0; h_index--) {
			context_one_network->backprop(error_one,
										  context_one_network_histories[h_index]);
			delete context_one_network_histories[h_index];
			error_two += context_one_network->input->errors[0];
			context_one_network->input->errors[0] = 0.0;

			vector<double> errors{error_two};
			context_two_network->backprop(errors,
										  context_two_network_histories[h_index]);
			delete context_two_network_histories[h_index];
			error_two += context_two_network->input->errors[0];
			context_two_network->input->errors[0] = 0.0;
		}

		if (iter_index % 20 == 0) {
			context_one_network->update();
			context_two_network->update();
			network->update();
		}
	}

	// temp
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(4, 0.0);
		double state_two = 0.0;

		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			cout << "a_index: " << a_index << endl;

			{
				cout << "contexts[sequence_index][a_index]: " << contexts[sequence_index][a_index] << endl;

				vector<double> inputs;
				inputs.push_back(state_two);
				inputs.push_back(contexts[sequence_index][a_index]);
				context_two_network->activate(inputs);
				state_two += context_two_network->output->acti_vals[0];

				cout << "context_two_network->output->acti_vals[0]: " << context_two_network->output->acti_vals[0] << endl;
			}

			{
				int action = sequences[sequence_index][a_index];
				cout << "action: " << action << endl;

				vector<double> inputs;
				inputs.push_back(state_two);
				for (int s_index = 0; s_index < 4; s_index++) {
					if (action == s_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}
				context_one_network->activate(inputs);
				for (int o_index = 0; o_index < 4; o_index++) {
					state_one[o_index] += context_one_network->output->acti_vals[o_index];
					cout << o_index << ": " << context_one_network->output->acti_vals[o_index] << endl;
				}
			}
		}

		network->activate(state_one);

		cout << "network->output->acti_vals[0]: " << network->output->acti_vals[0] << endl;
		cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;
	}
	// temp
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state_one(4, 0.0);
			double state_two = 0.0;

			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				{
					vector<double> inputs;
					inputs.push_back(state_two);
					inputs.push_back(contexts[h_index][a_index]);
					context_two_network->activate(inputs);
					state_two += context_two_network->output->acti_vals[0];
				}

				{
					int action = sequences[h_index][a_index];

					vector<double> inputs;
					inputs.push_back(state_two);
					for (int s_index = 0; s_index < 4; s_index++) {
						if (action == s_index) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}
					context_one_network->activate(inputs);
					for (int o_index = 0; o_index < 4; o_index++) {
						state_one[o_index] += context_one_network->output->acti_vals[o_index];
					}
				}
			}

			network->activate(state_one);

			sum_misguess += (target_vals[h_index] - network->output->acti_vals[0])
				* (target_vals[h_index] - network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	cout << "Done" << endl;
}
