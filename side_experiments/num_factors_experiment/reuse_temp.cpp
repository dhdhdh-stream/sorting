// - perhaps better to magnify output?

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

const double NETWORK_MIN_IMPACT = 0.03;

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

	vector<Network*> temp_networks_1;
	{
		vector<vector<double>> temp_inputs;
		vector<double> temp_target_vals;
		for (int h_index = 0; h_index < 1000; h_index++) {
			int sequence_index = sequence_distribution(generator);

			uniform_int_distribution<int> step_distribution(0, sequences[sequence_index].size()-1);
			int step_index = step_distribution(generator);

			int action = sequences[sequence_index][step_index];

			vector<double> inputs;
			for (int a_index = 0; a_index < 4; a_index++) {
				if (action == a_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}

			temp_inputs.push_back(inputs);
			temp_target_vals.push_back(target_vals[sequence_index]);
		}

		uniform_int_distribution<int> sample_distribution(0, temp_inputs.size()-1);
		for (int n_index = 0; n_index < 4; n_index++) {
			double sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)temp_target_vals.size(); h_index++) {
				sum_vals += temp_target_vals[h_index];
			}
			double val_average = sum_vals / (double)temp_target_vals.size();

			double sum_diffs = 0.0;
			for (int h_index = 0; h_index < (int)temp_target_vals.size(); h_index++) {
				sum_diffs += abs(temp_target_vals[h_index] - val_average);
			}
			double diff_average = sum_diffs / (double)temp_target_vals.size();

			for (int h_index = 0; h_index < (int)temp_target_vals.size(); h_index++) {
				temp_target_vals[h_index] = (temp_target_vals[h_index] - val_average) / diff_average;
			}

			// // temp
			// for (int h_index = 0; h_index < 20; h_index++) {
			// 	cout << temp_target_vals[h_index] << endl;
			// }

			Network* temp_network = new Network(LEAKY_LAYER, 4, 1);

			for (int iter_index = 0; iter_index < 300000; iter_index++) {
				if (iter_index % 10000 == 0) {
					cout << iter_index << endl;
				}

				int sample_index = sample_distribution(generator);
				temp_network->activate(temp_inputs[sample_index]);
				vector<double> errors{temp_target_vals[sample_index] - temp_network->output->acti_vals[0]};
				temp_network->backprop(errors);

				if (iter_index % 20 == 0) {
					temp_network->update();
				}
			}

			vector<double> temp_vals(temp_inputs.size());
			for (int h_index = 0; h_index < (int)temp_inputs.size(); h_index++) {
				temp_network->activate(temp_inputs[h_index]);
				temp_vals[h_index] = temp_network->output->acti_vals[0];
			}

			double sum_temp_vals = 0.0;
			for (int h_index = 0; h_index < (int)temp_inputs.size(); h_index++) {
				sum_temp_vals += temp_vals[h_index];
			}
			double temp_val_average = sum_temp_vals / (double)temp_inputs.size();

			double sum_temp_diffs = 0.0;
			for (int h_index = 0; h_index < (int)temp_inputs.size(); h_index++) {
				sum_temp_diffs += abs(temp_vals[h_index] - temp_val_average);
			}
			double temp_diff_average = sum_temp_diffs / (double)temp_inputs.size();

			if (temp_diff_average < NETWORK_MIN_IMPACT) {
				delete temp_network;
				break;
			}

			for (int h_index = 0; h_index < (int)temp_inputs.size(); h_index++) {
				temp_target_vals[h_index] -= temp_vals[h_index];
			}
			temp_networks_1.push_back(temp_network);
		}

		// // temp
		// for (int h_index = 0; h_index < 20; h_index++) {
		// 	cout << h_index << endl;
		// 	for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
		// 		temp_networks_1[n_index]->activate(temp_inputs[h_index]);
		// 		cout << n_index << ": " << temp_networks_1[n_index]->output->acti_vals[0] << endl;
		// 	}
		// }
	}

	Network* final_network = new Network(LEAKY_LAYER, temp_networks_1.size(), 1);

	for (int iter_index = 0; iter_index < 100000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<double> state_one(temp_networks_1.size(), 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			vector<double> inputs;
			for (int a_index = 0; a_index < 4; a_index++) {
				if (action == a_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}

			for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
				temp_networks_1[n_index]->activate(inputs);
				state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
			}
		}

		final_network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
		final_network->backprop(final_errors);

		if (iter_index % 20 == 0) {
			final_network->update();
		}
	}

	for (int i_index = 0; i_index < (int)temp_networks_1.size(); i_index++) {
		final_network->input->errors[i_index] = 0.0;
	}
	for (int iter_index = 0; iter_index < 200000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		vector<vector<NetworkHistory*>> state_one_network_histories;

		vector<double> state_one(temp_networks_1.size(), 0.0);
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			int action = sequences[sequence_index][a_index];

			vector<double> inputs;
			for (int a_index = 0; a_index < 4; a_index++) {
				if (action == a_index) {
					inputs.push_back(1.0);
				} else {
					inputs.push_back(0.0);
				}
			}

			vector<NetworkHistory*> curr_network_histories(temp_networks_1.size());
			for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
				NetworkHistory* network_history = new NetworkHistory();
				temp_networks_1[n_index]->activate(inputs,
												   network_history);
				curr_network_histories[n_index] = network_history;
				state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
			}
			state_one_network_histories.push_back(curr_network_histories);
		}

		final_network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
		final_network->backprop(final_errors);

		vector<double> error_one(temp_networks_1.size(), 0.0);
		for (int i_index = 0; i_index < (int)temp_networks_1.size(); i_index++) {
			error_one[i_index] += final_network->input->errors[i_index];
			final_network->input->errors[i_index] = 0.0;
		}

		for (int h_index = (int)state_one_network_histories.size()-1; h_index >= 0; h_index--) {
			for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
				vector<double> errors{error_one[n_index]};
				temp_networks_1[n_index]->backprop(errors,
												   state_one_network_histories[h_index][n_index]);
				delete state_one_network_histories[h_index][n_index];
			}
		}

		if (iter_index % 20 == 0) {
			final_network->update();
			for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
				temp_networks_1[n_index]->update();
			}
		}
	}

	// // temp
	// for (int iter_index = 0; iter_index < 40; iter_index++) {
	// 	cout << iter_index << endl;

	// 	int sequence_index = sequence_distribution(generator);

	// 	vector<double> state_one(temp_networks_1.size(), 0.0);
	// 	for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
	// 		cout << "a_index: " << a_index << endl;

	// 		int action = sequences[sequence_index][a_index];
	// 		cout << "action: " << action << endl;

	// 		vector<double> inputs;
	// 		for (int a_index = 0; a_index < 4; a_index++) {
	// 			if (action == a_index) {
	// 				inputs.push_back(1.0);
	// 			} else {
	// 				inputs.push_back(0.0);
	// 			}
	// 		}

	// 		for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
	// 			temp_networks_1[n_index]->activate(inputs);
	// 			state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
	// 			cout << n_index << ": " << temp_networks_1[n_index]->output->acti_vals[0] << endl;
	// 		}
	// 	}

	// 	final_network->activate(state_one);

	// 	cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
	// 	cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;

	// 	cout << endl;
	// }
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state_one(temp_networks_1.size(), 0.0);
			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				int action = sequences[h_index][a_index];

				vector<double> inputs;
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
					temp_networks_1[n_index]->activate(inputs);
					state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
				}
			}

			final_network->activate(state_one);

			sum_misguess += (target_vals[h_index] - final_network->output->acti_vals[0])
				* (target_vals[h_index] - final_network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	Network* temp_network_2 = new Network(LEAKY_LAYER, 2, 1);
	{
		vector<vector<double>> temp_inputs;
		vector<double> temp_target_vals;
		for (int h_index = 0; h_index < 1000; h_index++) {
			int sequence_index = sequence_distribution(generator);

			vector<double> state_one(temp_networks_1.size(), 0.0);
			for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
				int action = sequences[sequence_index][a_index];

				vector<double> inputs;
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
					temp_networks_1[n_index]->activate(inputs);
					state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
				}
			}

			final_network->activate(state_one);

			vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
			final_network->backprop_through(final_errors);

			vector<double> error_one(temp_networks_1.size(), 0.0);
			for (int i_index = 0; i_index < (int)temp_networks_1.size(); i_index++) {
				error_one[i_index] += final_network->input->errors[i_index];
				final_network->input->errors[i_index] = 0.0;
			}

			uniform_int_distribution<int> step_distribution(0, sequences[sequence_index].size()-1);
			int step_index = step_distribution(generator);

			vector<double> inputs{0.0, (double)contexts[sequence_index][step_index]};

			temp_inputs.push_back(inputs);
			temp_target_vals.push_back(error_one[0]);
		}

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)temp_target_vals.size(); h_index++) {
			sum_vals += temp_target_vals[h_index];
		}
		double val_average = sum_vals / (double)temp_target_vals.size();

		double sum_diffs = 0.0;
		for (int h_index = 0; h_index < (int)temp_target_vals.size(); h_index++) {
			sum_diffs += abs(temp_target_vals[h_index] - val_average);
		}
		double diff_average = sum_diffs / (double)temp_target_vals.size();

		for (int h_index = 0; h_index < (int)temp_target_vals.size(); h_index++) {
			temp_target_vals[h_index] = (temp_target_vals[h_index] - val_average) / diff_average;
		}

		uniform_int_distribution<int> sample_distribution(0, temp_inputs.size()-1);
		for (int iter_index = 0; iter_index < 300000; iter_index++) {
			if (iter_index % 10000 == 0) {
				cout << iter_index << endl;
			}

			int sample_index = sample_distribution(generator);
			temp_network_2->activate(temp_inputs[sample_index]);
			vector<double> errors{temp_target_vals[sample_index] - temp_network_2->output->acti_vals[0]};
			temp_network_2->backprop(errors);

			if (iter_index % 20 == 0) {
				temp_network_2->update();
			}
		}

		// temp
		cout << "temp_network_2->output->acti_vals[0]:" << endl;
		for (int h_index = 0; h_index < 10; h_index++) {
			temp_network_2->activate(temp_inputs[h_index]);
			cout << temp_network_2->output->acti_vals[0] << endl;
		}
	}

	Network* state_two_existing_network = new Network(LEAKY_LAYER, 5, 1);

	// for (int iter_index = 0; iter_index < 100000; iter_index++) {
	for (int iter_index = 0; iter_index < 200000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double ratio = 1.0 - iter_index / 200000.0;

		vector<NetworkHistory*> state_two_existing_network_histories;

		vector<double> state_one(temp_networks_1.size(), 0.0);
		double state_two = 0.0;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			{
				state_two *= 0.5;

				vector<double> inputs{state_two, (double)contexts[sequence_index][a_index]};
				temp_network_2->activate(inputs);
				// state_two += temp_network_2->output->acti_vals[0];
				state_two += 10.0 * temp_network_2->output->acti_vals[0];
			}

			int action = sequences[sequence_index][a_index];

			{
				vector<double> inputs;
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
					temp_networks_1[n_index]->activate(inputs);
					state_one[n_index] += ratio * temp_networks_1[n_index]->output->acti_vals[0];
				}
			}

			{
				vector<double> inputs{state_two};
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				NetworkHistory* network_history = new NetworkHistory();
				state_two_existing_network->activate(inputs,
													 network_history);
				state_two_existing_network_histories.push_back(network_history);
				state_one[0] += state_two_existing_network->output->acti_vals[0];
			}
		}

		final_network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
		final_network->backprop_through(final_errors);

		vector<double> error_one(temp_networks_1.size(), 0.0);
		for (int i_index = 0; i_index < (int)temp_networks_1.size(); i_index++) {
			error_one[i_index] += final_network->input->errors[i_index];
			final_network->input->errors[i_index] = 0.0;
		}

		for (int h_index = (int)state_two_existing_network_histories.size()-1; h_index >= 0; h_index--) {
			state_two_existing_network->backprop(error_one,
												 state_two_existing_network_histories[h_index]);
			delete state_two_existing_network_histories[h_index];
		}

		if (iter_index % 20 == 0) {
			state_two_existing_network->update();
		}
	}

	for (int i_index = 0; i_index < (int)state_two_existing_network->input->errors.size(); i_index++) {
		state_two_existing_network->input->errors[i_index] = 0.0;
	}
	for (int iter_index = 0; iter_index < 200000; iter_index++) {
		if (iter_index % 10000 == 0) {
			cout << iter_index << endl;
		}

		int sequence_index = sequence_distribution(generator);

		double factor = 0.5 + 0.5 * iter_index / 200000.0;

		// vector<vector<NetworkHistory*>> state_one_network_histories;
		vector<NetworkHistory*> state_two_new_network_histories;
		vector<NetworkHistory*> state_two_existing_network_histories;

		vector<double> state_one(temp_networks_1.size(), 0.0);
		double state_two = 0.0;
		for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
			{
				state_two *= factor;

				vector<double> inputs{state_two, (double)contexts[sequence_index][a_index]};
				NetworkHistory* network_history = new NetworkHistory();
				temp_network_2->activate(inputs,
										 network_history);
				state_two_new_network_histories.push_back(network_history);
				// state_two += temp_network_2->output->acti_vals[0];
				state_two += 10.0 * temp_network_2->output->acti_vals[0];
			}

			int action = sequences[sequence_index][a_index];

			// {
			// 	vector<double> inputs;
			// 	for (int a_index = 0; a_index < 4; a_index++) {
			// 		if (action == a_index) {
			// 			inputs.push_back(1.0);
			// 		} else {
			// 			inputs.push_back(0.0);
			// 		}
			// 	}

			// 	vector<NetworkHistory*> curr_network_histories(temp_networks_1.size());
			// 	for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
			// 		NetworkHistory* network_history = new NetworkHistory();
			// 		temp_networks_1[n_index]->activate(inputs,
			// 										   network_history);
			// 		curr_network_histories[n_index] = network_history;
			// 		state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
			// 	}
			// 	state_one_network_histories.push_back(curr_network_histories);
			// }

			{
				vector<double> inputs{state_two};
				for (int a_index = 0; a_index < 4; a_index++) {
					if (action == a_index) {
						inputs.push_back(1.0);
					} else {
						inputs.push_back(0.0);
					}
				}

				NetworkHistory* network_history = new NetworkHistory();
				state_two_existing_network->activate(inputs,
													 network_history);
				state_two_existing_network_histories.push_back(network_history);
				state_one[0] += state_two_existing_network->output->acti_vals[0];
			}
		}

		final_network->activate(state_one);

		vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
		final_network->backprop(final_errors);

		vector<double> error_one(temp_networks_1.size(), 0.0);
		for (int i_index = 0; i_index < (int)temp_networks_1.size(); i_index++) {
			error_one[i_index] += final_network->input->errors[i_index];
			final_network->input->errors[i_index] = 0.0;
		}
		double error_two = 0.0;

		for (int h_index = (int)sequences[sequence_index].size()-1; h_index >= 0; h_index--) {
			// for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
			// 	vector<double> errors{error_one[n_index]};
			// 	temp_networks_1[n_index]->backprop(errors,
			// 									   state_one_network_histories[h_index][n_index]);
			// 	delete state_one_network_histories[h_index][n_index];
			// }

			state_two_existing_network->backprop(error_one,
												 state_two_existing_network_histories[h_index]);
			delete state_two_existing_network_histories[h_index];
			error_two += state_two_existing_network->input->errors[0];
			state_two_existing_network->input->errors[0] = 0.0;

			{
				vector<double> errors{error_two};
				temp_network_2->backprop(errors,
										 state_two_new_network_histories[h_index]);
				delete state_two_new_network_histories[h_index];
				error_two += temp_network_2->input->errors[0];
				temp_network_2->input->errors[0] = 0.0;
			}
		}

		if (iter_index % 20 == 0) {
			final_network->update();
			// for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
			// 	temp_networks_1[n_index]->update();
			// }
			temp_network_2->update();
			state_two_existing_network->update();
		}
	}
	// for (int iter_index = 0; iter_index < 100000; iter_index++) {
	// 	if (iter_index % 10000 == 0) {
	// 		cout << iter_index << endl;
	// 	}

	// 	int sequence_index = sequence_distribution(generator);

	// 	// vector<vector<NetworkHistory*>> state_one_network_histories;
	// 	vector<NetworkHistory*> state_two_new_network_histories;
	// 	vector<NetworkHistory*> state_two_existing_network_histories;

	// 	vector<double> state_one(temp_networks_1.size(), 0.0);
	// 	double state_two = 0.0;
	// 	for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
	// 		{
	// 			vector<double> inputs{state_two, (double)contexts[sequence_index][a_index]};
	// 			NetworkHistory* network_history = new NetworkHistory();
	// 			temp_network_2->activate(inputs,
	// 									 network_history);
	// 			state_two_new_network_histories.push_back(network_history);
	// 			state_two += temp_network_2->output->acti_vals[0];
	// 		}

	// 		int action = sequences[sequence_index][a_index];

	// 		// {
	// 		// 	vector<double> inputs;
	// 		// 	for (int a_index = 0; a_index < 4; a_index++) {
	// 		// 		if (action == a_index) {
	// 		// 			inputs.push_back(1.0);
	// 		// 		} else {
	// 		// 			inputs.push_back(0.0);
	// 		// 		}
	// 		// 	}

	// 		// 	vector<NetworkHistory*> curr_network_histories(temp_networks_1.size());
	// 		// 	for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
	// 		// 		NetworkHistory* network_history = new NetworkHistory();
	// 		// 		temp_networks_1[n_index]->activate(inputs,
	// 		// 										   network_history);
	// 		// 		curr_network_histories[n_index] = network_history;
	// 		// 		state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
	// 		// 	}
	// 		// 	state_one_network_histories.push_back(curr_network_histories);
	// 		// }

	// 		{
	// 			vector<double> inputs{state_two};
	// 			for (int a_index = 0; a_index < 4; a_index++) {
	// 				if (action == a_index) {
	// 					inputs.push_back(1.0);
	// 				} else {
	// 					inputs.push_back(0.0);
	// 				}
	// 			}

	// 			NetworkHistory* network_history = new NetworkHistory();
	// 			state_two_existing_network->activate(inputs,
	// 												 network_history);
	// 			state_two_existing_network_histories.push_back(network_history);
	// 			state_one[0] += state_two_existing_network->output->acti_vals[0];
	// 		}
	// 	}

	// 	final_network->activate(state_one);

	// 	vector<double> final_errors{target_vals[sequence_index] - final_network->output->acti_vals[0]};
	// 	final_network->backprop(final_errors);

	// 	vector<double> error_one(temp_networks_1.size(), 0.0);
	// 	for (int i_index = 0; i_index < (int)temp_networks_1.size(); i_index++) {
	// 		error_one[i_index] += final_network->input->errors[i_index];
	// 		final_network->input->errors[i_index] = 0.0;
	// 	}
	// 	double error_two = 0.0;

	// 	for (int h_index = (int)sequences[sequence_index].size()-1; h_index >= 0; h_index--) {
	// 		// for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
	// 		// 	vector<double> errors{error_one[n_index]};
	// 		// 	temp_networks_1[n_index]->backprop(errors,
	// 		// 									   state_one_network_histories[h_index][n_index]);
	// 		// 	delete state_one_network_histories[h_index][n_index];
	// 		// }

	// 		state_two_existing_network->backprop(error_one,
	// 											 state_two_existing_network_histories[h_index]);
	// 		delete state_two_existing_network_histories[h_index];
	// 		error_two += state_two_existing_network->input->errors[0];
	// 		state_two_existing_network->input->errors[0] = 0.0;

	// 		{
	// 			vector<double> errors{error_two};
	// 			temp_network_2->backprop(errors,
	// 									 state_two_new_network_histories[h_index]);
	// 			delete state_two_new_network_histories[h_index];
	// 			error_two += temp_network_2->input->errors[0];
	// 			temp_network_2->input->errors[0] = 0.0;
	// 		}
	// 	}

	// 	if (iter_index % 20 == 0) {
	// 		final_network->update();
	// 		// for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
	// 		// 	temp_networks_1[n_index]->update();
	// 		// }
	// 		temp_network_2->update();
	// 		state_two_existing_network->update();
	// 	}
	// }

	// temp
	// for (int iter_index = 0; iter_index < 20; iter_index++) {
	// 	cout << iter_index << endl;

	// 	int sequence_index = sequence_distribution(generator);

	// 	vector<double> state_one(temp_networks_1.size(), 0.0);
	// 	double state_two = 0.0;
	// 	for (int a_index = 0; a_index < (int)sequences[sequence_index].size(); a_index++) {
	// 		cout << "a_index: " << a_index << endl;

	// 		cout << "contexts[sequence_index][a_index]: " << contexts[sequence_index][a_index] << endl;

	// 		{
	// 			vector<double> inputs{state_two, (double)contexts[sequence_index][a_index]};
	// 			temp_network_2->activate(inputs);
	// 			state_two += temp_network_2->output->acti_vals[0];
	// 			cout << "temp_network_2->output->acti_vals[0]: " << temp_network_2->output->acti_vals[0] << endl;
	// 			cout << "state_two: " << state_two << endl;
	// 		}

	// 		int action = sequences[sequence_index][a_index];
	// 		cout << "action: " << action << endl;

	// 		// {
	// 		// 	vector<double> inputs;
	// 		// 	for (int a_index = 0; a_index < 4; a_index++) {
	// 		// 		if (action == a_index) {
	// 		// 			inputs.push_back(1.0);
	// 		// 		} else {
	// 		// 			inputs.push_back(0.0);
	// 		// 		}
	// 		// 	}

	// 		// 	for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
	// 		// 		temp_networks_1[n_index]->activate(inputs);
	// 		// 		state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
	// 		// 	}
	// 		// }

	// 		{
	// 			vector<double> inputs{state_two};
	// 			for (int a_index = 0; a_index < 4; a_index++) {
	// 				if (action == a_index) {
	// 					inputs.push_back(1.0);
	// 				} else {
	// 					inputs.push_back(0.0);
	// 				}
	// 			}

	// 			state_two_existing_network->activate(inputs);
	// 			state_one[0] += state_two_existing_network->output->acti_vals[0];
	// 			cout << "state_two_existing_network->output->acti_vals[0]: " << state_two_existing_network->output->acti_vals[0] << endl;
	// 			cout << "state_one[0]: " << state_one[0] << endl;
	// 		}
	// 	}

	// 	final_network->activate(state_one);

	// 	cout << "final_network->output->acti_vals[0]: " << final_network->output->acti_vals[0] << endl;
	// 	cout << "target_vals[sequence_index]: " << target_vals[sequence_index] << endl;
	// }
	{
		double sum_misguess = 0.0;
		for (int h_index = 0; h_index < NUM_SAMPLES; h_index++) {
			vector<double> state_one(temp_networks_1.size(), 0.0);
			double state_two = 0.0;
			for (int a_index = 0; a_index < (int)sequences[h_index].size(); a_index++) {
				{
					// // temp
					// state_two *= 0.5;

					vector<double> inputs{state_two, (double)contexts[h_index][a_index]};
					temp_network_2->activate(inputs);
					// state_two += temp_network_2->output->acti_vals[0];
					state_two += 10.0 * temp_network_2->output->acti_vals[0];
				}

				int action = sequences[h_index][a_index];

				// {
				// 	vector<double> inputs;
				// 	for (int a_index = 0; a_index < 4; a_index++) {
				// 		if (action == a_index) {
				// 			inputs.push_back(1.0);
				// 		} else {
				// 			inputs.push_back(0.0);
				// 		}
				// 	}

				// 	for (int n_index = 0; n_index < (int)temp_networks_1.size(); n_index++) {
				// 		temp_networks_1[n_index]->activate(inputs);
				// 		state_one[n_index] += temp_networks_1[n_index]->output->acti_vals[0];
				// 	}
				// }

				{
					vector<double> inputs{state_two};
					for (int a_index = 0; a_index < 4; a_index++) {
						if (action == a_index) {
							inputs.push_back(1.0);
						} else {
							inputs.push_back(0.0);
						}
					}

					state_two_existing_network->activate(inputs);
					state_one[0] += state_two_existing_network->output->acti_vals[0];
				}
			}

			final_network->activate(state_one);

			sum_misguess += (target_vals[h_index] - final_network->output->acti_vals[0])
				* (target_vals[h_index] - final_network->output->acti_vals[0]);
		}
		double misguess_average = sum_misguess / (double)NUM_SAMPLES;
		cout << "misguess_average: " << misguess_average << endl;
	}

	cout << "Done" << endl;
}
