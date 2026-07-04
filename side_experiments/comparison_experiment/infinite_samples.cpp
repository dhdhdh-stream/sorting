// works sometimes

// needs large state size
// probably to carry a large number of possibilities to end to work with

// - everything is so expensive for recurrent networks
//   - paying attention to anything means have to find a way to ignore everything else

// TODO: have custom networks for every action node
// - but for predict, can have generic network

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

const int NUM_STATES = 8;
// const int NUM_STATES = 4;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<int> context_input_sizes{NUM_STATES, 1};
	Network* context_network = new Network(context_input_sizes, NUM_STATES);
	double context_hidden_1_average_max_update = 0.0;
	double context_hidden_2_average_max_update = 0.0;
	double context_hidden_3_average_max_update = 0.0;
	double context_output_average_max_update = 0.0;
	vector<int> action_input_sizes{NUM_STATES, 4};
	Network* action_network = new Network(action_input_sizes, NUM_STATES);
	double action_hidden_1_average_max_update = 0.0;
	double action_hidden_2_average_max_update = 0.0;
	double action_hidden_3_average_max_update = 0.0;
	double action_output_average_max_update = 0.0;
	vector<int> target_val_input_sizes{NUM_STATES};
	Network* target_val_network = new Network(target_val_input_sizes, 1);
	double target_val_hidden_1_average_max_update = 0.0;
	double target_val_hidden_2_average_max_update = 0.0;
	double target_val_hidden_3_average_max_update = 0.0;
	double target_val_output_average_max_update = 0.0;

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, 3);
	uniform_int_distribution<int> context_distribution(0, 4);
	// for (int iter_index = 0; iter_index < 300000; iter_index++) {
	for (int iter_index = 0; iter_index < 1000000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		vector<int> curr_contexts;
		vector<int> curr_actions;

		int curr_loc = 0;

		int start_num_actions = num_actions_distribution(generator);;
		for (int a_index = 0; a_index < start_num_actions; a_index++) {
			curr_contexts.push_back(0);
			
			int action = action_distribution(generator);
			switch (action) {
			case 0:
				curr_loc++;
				break;
			case 1:
				curr_loc--;
				break;
			}
			curr_actions.push_back(action);
		}

		curr_contexts.push_back(1);
		int start_loc = curr_loc;

		int end_num_actions = num_actions_distribution(generator);
		for (int a_index = 0; a_index < end_num_actions; a_index++) {
			int action = action_distribution(generator);
			switch (action) {
			case 0:
				curr_loc++;
				break;
			case 1:
				curr_loc--;
				break;
			}
			curr_actions.push_back(action);

			curr_contexts.push_back(0);
		}

		int action = action_distribution(generator);
		switch (action) {
		case 0:
			curr_loc++;
			break;
		case 1:
			curr_loc--;
			break;
		}
		curr_actions.push_back(action);

		double target_val;
		if (curr_loc == start_loc) {
			target_val = 10.0;
		} else {
			target_val = -10.0;
		}

		vector<NetworkHistory*> context_network_histories;
		vector<NetworkHistory*> action_network_histories;

		vector<double> state(NUM_STATES, 0.0);
		for (int a_index = 0; a_index < (int)curr_contexts.size(); a_index++) {
			{
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					context_network->inputs[0]->acti_vals(s_index) = state[s_index];
				}
				context_network->inputs[1]->acti_vals(0) = curr_contexts[a_index];
				NetworkHistory* network_history = new NetworkHistory(context_network);
				context_network->activate(network_history);
				context_network_histories.push_back(network_history);
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					state[s_index] += context_network->output->acti_vals(s_index);
				}
			}

			{
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					action_network->inputs[0]->acti_vals(s_index) = state[s_index];
				}
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == curr_actions[a_index]) {
						action_network->inputs[1]->acti_vals(i_index) = 1.0;
					} else {
						action_network->inputs[1]->acti_vals(i_index) = 0.0;
					}
				}
				NetworkHistory* network_history = new NetworkHistory(action_network);
				action_network->activate(network_history);
				action_network_histories.push_back(network_history);
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					state[s_index] += action_network->output->acti_vals(s_index);
				}
			}
		}

		for (int s_index = 0; s_index < NUM_STATES; s_index++) {
			target_val_network->inputs[0]->acti_vals(s_index) = state[s_index];
		}
		target_val_network->activate();

		target_val_network->output->errors(0) = target_val - target_val_network->output->acti_vals(0);
		target_val_network->backprop();

		vector<double> state_errors(NUM_STATES, 0.0);

		for (int i_index = 0; i_index < NUM_STATES; i_index++) {
			state_errors[i_index] += target_val_network->inputs[0]->errors(i_index);
			target_val_network->inputs[0]->errors(i_index) = 0.0;
		}

		for (int h_index = (int)curr_contexts.size()-1; h_index >= 0; h_index--) {
			for (int i_index = 0; i_index < NUM_STATES; i_index++) {
				action_network->output->errors(i_index) = state_errors[i_index];
			}
			action_network->backprop(action_network_histories[h_index]);
			delete action_network_histories[h_index];
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				state_errors[s_index] += action_network->inputs[0]->errors(s_index);
				action_network->inputs[0]->errors(s_index) = 0.0;
			}

			for (int i_index = 0; i_index < NUM_STATES; i_index++) {
				context_network->output->errors(i_index) = state_errors[i_index];
			}
			context_network->backprop(context_network_histories[h_index]);
			delete context_network_histories[h_index];
			for (int s_index = 0; s_index < NUM_STATES; s_index++) {
				state_errors[s_index] += context_network->inputs[0]->errors(s_index);
				context_network->inputs[0]->errors(s_index) = 0.0;
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
			target_val_network->init_update(target_val_hidden_1_average_max_update,
											target_val_hidden_2_average_max_update,
											target_val_hidden_3_average_max_update,
											target_val_output_average_max_update);
		}
	}

	// temp
	for (int iter_index = 0; iter_index < 40; iter_index++) {
		cout << iter_index << endl;

		vector<int> curr_contexts;
		vector<int> curr_actions;

		int curr_loc = 0;

		int start_num_actions = num_actions_distribution(generator);;
		for (int a_index = 0; a_index < start_num_actions; a_index++) {
			curr_contexts.push_back(0);
			
			int action = action_distribution(generator);
			switch (action) {
			case 0:
				curr_loc++;
				break;
			case 1:
				curr_loc--;
				break;
			}
			curr_actions.push_back(action);
		}

		curr_contexts.push_back(1);
		int start_loc = curr_loc;

		int end_num_actions = num_actions_distribution(generator);
		for (int a_index = 0; a_index < end_num_actions; a_index++) {
			int action = action_distribution(generator);
			switch (action) {
			case 0:
				curr_loc++;
				break;
			case 1:
				curr_loc--;
				break;
			}
			curr_actions.push_back(action);

			curr_contexts.push_back(0);
		}

		int action = action_distribution(generator);
		switch (action) {
		case 0:
			curr_loc++;
			break;
		case 1:
			curr_loc--;
			break;
		}
		curr_actions.push_back(action);

		double target_val;
		if (curr_loc == start_loc) {
			target_val = 10.0;
		} else {
			target_val = -10.0;
		}

		vector<double> state(NUM_STATES, 0.0);
		for (int a_index = 0; a_index < (int)curr_contexts.size(); a_index++) {
			{
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					context_network->inputs[0]->acti_vals(s_index) = state[s_index];
				}
				context_network->inputs[1]->acti_vals(0) = curr_contexts[a_index];
				context_network->activate();
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					state[s_index] += context_network->output->acti_vals(s_index);
				}
			}

			{
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					action_network->inputs[0]->acti_vals(s_index) = state[s_index];
				}
				for (int i_index = 0; i_index < 4; i_index++) {
					if (i_index == curr_actions[a_index]) {
						action_network->inputs[1]->acti_vals(i_index) = 1.0;
					} else {
						action_network->inputs[1]->acti_vals(i_index) = 0.0;
					}
				}
				action_network->activate();
				for (int s_index = 0; s_index < NUM_STATES; s_index++) {
					state[s_index] += action_network->output->acti_vals(s_index);
				}
			}
		}

		for (int s_index = 0; s_index < NUM_STATES; s_index++) {
			target_val_network->inputs[0]->acti_vals(s_index) = state[s_index];
		}
		target_val_network->activate();

		cout << "target_val_network->output->acti_vals(0): " << target_val_network->output->acti_vals(0) << endl;
		cout << "target_val: " << target_val << endl;

		cout << endl;
	}

	cout << "Done" << endl;
}
