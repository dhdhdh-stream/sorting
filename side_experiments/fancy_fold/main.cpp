#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"

using namespace std;

default_random_engine generator;

int fold_index = 0;
vector<int> state_network_num_inputs;	// starting from back
vector<int> state_network_num_compression;
vector<int> state_network_num_outputs_added;
vector<int> state_network_num_outputs;	// including new outputs added, starting from back, limited by inputs
vector<Network*> state_networks;

// don't think about branching yet
void run_step(vector<double>& state_vals,
			  double observation,
			  int step_index) {
	vector<double> inputs;
	inputs.push_back(observation);
	for (int i = 0; i < state_network_num_inputs[step_index]; i++) {
		inputs.push_back(state_vals[state_vals.size()-1-i]);
	}

	state_networks[step_index]->activate(inputs);

	if (state_network_num_compression[step_index] > 0) {
		for (int i = 0; i < state_network_num_compression[step_index]; i++) {
			state_vals.pop_back();
		}
		state_vals.back() = state_networks[step_index]->output->acti_vals[0];
	} else {
		for (int i = 0; i < state_network_num_outputs_added[step_index]; i++) {
			state_vals.push_back(0.0);
		}

		for (int i = 0; i < state_network_num_outputs[step_index]; i++) {
			state_vals[state_vals.size()-1-i] = state_networks[step_index]->output->acti_vals[i];
		}
	}
}

void backprop_step(vector<double>& state_errors,
				   int step_index) {
	vector<double> errors;
	for (int i = 0; i < state_network_num_outputs[step_index]; i++) {
		errors.push_back(state_errors[state_errors.size()-1-i]);
	}

	state_networks[step_index]->backprop(errors);

	if (state_network_num_outputs_added[step_index] > 0) {
		for (int i = 0; i < state_network_num_outputs_added[step_index]; i++) {
			state_errors.pop_back();
		}
	} else if (state_network_num_compression[step_index] > 0) {
		for (int i = 0; i < state_network_num_compression[step_index]; i++) {
			state_errors.push_back(0.0);
		}
	}

	for (int i = 0; i < state_network_num_inputs[step_index]; i++) {
		state_errors[state_errors.size()-1-i] = state_networks[step_index]->input->errors[1+i];
		state_networks[step_index]->input->errors[1+i] = 0.0;
	}
}

void run_test_network(vector<double>& state_vals,
					  vector<double>& next_state_vals,
					  double observation,
					  Network* network,
					  int num_input,
					  int num_compression,
					  int num_output_added,
					  int num_output) {
	vector<double> inputs;
	inputs.push_back(observation);
	for (int i = 0; i < num_input; i++) {
		inputs.push_back(state_vals[state_vals.size()-1-i]);
	}

	network->activate(inputs);

	next_state_vals = state_vals;

	if (num_compression > 0) {
		for (int i = 0; i < num_compression; i++) {
			next_state_vals.pop_back();
		}
		next_state_vals.back() = network->output->acti_vals[0];
	} else {
		for (int i = 0; i < num_output_added; i++) {
			next_state_vals.push_back(0.0);
		}

		for (int i = 0; i < num_output; i++) {
			next_state_vals[next_state_vals.size()-1-i] = network->output->acti_vals[i];
		}
	}
}

void backprop_test_network(vector<double>& state_errors,
						   Network* network,
						   int num_input,
						   int num_compression,
						   int num_output_added,
						   int num_output) {
	vector<double> errors;
	for (int i = 0; i < num_output; i++) {
		errors.push_back(state_errors[state_errors.size()-1-i]);
	}

	network->backprop(errors);

	if (num_output_added > 0) {
		for (int i = 0; i < num_output_added; i++) {
			state_errors.pop_back();
		}
	} else if (num_compression > 0) {
		for (int i = 0; i < num_compression; i++) {
			state_errors.push_back(0.0);
		}
	}

	for (int i = 0; i < num_input; i++) {
		state_errors[state_errors.size()-1-i] = network->input->errors[1+i];
		network->input->errors[1+i] = 0.0;
	}
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// FoldNetwork fold_network(11, 1);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {

	// 		double flat_inputs[11];
	// 		int flat_input_counter = 0;
	// 		bool activated[11];
	// 		for (int i = 0; i < 11; i++) {
	// 			activated[i] = true;
	// 		}

	// 		// first block
	// 		int first_block_sum = 0;
	// 		for (int i = 0; i < 2; i++) {
	// 			if (rand()%2 == 0) {
	// 				flat_inputs[flat_input_counter] = 1.0;
	// 				flat_input_counter++;
	// 				first_block_sum++;
	// 			} else {
	// 				flat_inputs[flat_input_counter] = 0.0;
	// 				flat_input_counter++;
	// 			}
	// 		}

	// 		// dud
	// 		flat_inputs[flat_input_counter] = rand()%2;
	// 		flat_input_counter++;

	// 		for (int i = 0; i < 2; i++) {
	// 			if (rand()%2 == 0) {
	// 				flat_inputs[flat_input_counter] = 1.0;
	// 				flat_input_counter++;
	// 				first_block_sum++;
	// 			} else {
	// 				flat_inputs[flat_input_counter] = 0.0;
	// 				flat_input_counter++;
	// 			}
	// 		}

	// 		// dud
	// 		flat_inputs[flat_input_counter] = rand()%2;
	// 		flat_input_counter++;

	// 		// second block
	// 		int second_block_sum = 0;
	// 		for (int i = 0; i < 4; i++) {
	// 			if (rand()%2 == 0) {
	// 				flat_inputs[flat_input_counter] = 1.0;
	// 				flat_input_counter++;
	// 				second_block_sum++;
	// 			} else {
	// 				flat_inputs[flat_input_counter] = 0.0;
	// 				flat_input_counter++;
	// 			}
	// 		}

	// 		// dud
	// 		flat_inputs[flat_input_counter] = rand()%2;
	// 		flat_input_counter++;

	// 		int total_sum = 0;
	// 		bool first_block_is_even = (first_block_sum%2 == 0);
	// 		if (first_block_is_even) {
	// 			total_sum += 7;
	// 		}
	// 		bool second_block_is_even = (second_block_sum%2 == 0);
	// 		if (second_block_is_even) {
	// 			total_sum += 5;
	// 		}

	// 		vector<double> obs;
	// 		obs.push_back(rand()%2);

	// 		fold_network.activate(flat_inputs,
	// 							  activated,
	// 							  obs);

	// 		vector<double> errors;
	// 		errors.push_back(total_sum - fold_network.output->acti_vals[0]);
	// 		sum_error += abs(errors[0]);

	// 		fold_network.backprop(errors);
	// 	}
	// }

	// ofstream output_file;
	// output_file.open("saves/fold_network.txt");
	// fold_network.save(output_file);
	// output_file.close();

	ifstream input_file;
	input_file.open("saves/fold_network.txt");
	FoldNetwork* fold_network = new FoldNetwork(11, 1, input_file);
	input_file.close();

	double average_error = 0.0;
	for (int iter_index = 0; iter_index < 10000; iter_index++) {
		double flat_inputs[11];
		int flat_input_counter = 0;
		bool activated[11];
		for (int i = 0; i < 11; i++) {
			activated[i] = true;
		}

		// first block
		int first_block_sum = 0;
		for (int i = 0; i < 2; i++) {
			if (rand()%2 == 0) {
				flat_inputs[flat_input_counter] = 1.0;
				flat_input_counter++;
				first_block_sum++;
			} else {
				flat_inputs[flat_input_counter] = 0.0;
				flat_input_counter++;
			}
		}

		// dud
		flat_inputs[flat_input_counter] = rand()%2;
		flat_input_counter++;

		for (int i = 0; i < 2; i++) {
			if (rand()%2 == 0) {
				flat_inputs[flat_input_counter] = 1.0;
				flat_input_counter++;
				first_block_sum++;
			} else {
				flat_inputs[flat_input_counter] = 0.0;
				flat_input_counter++;
			}
		}

		// dud
		flat_inputs[flat_input_counter] = rand()%2;
		flat_input_counter++;

		// second block
		int second_block_sum = 0;
		for (int i = 0; i < 4; i++) {
			if (rand()%2 == 0) {
				flat_inputs[flat_input_counter] = 1.0;
				flat_input_counter++;
				second_block_sum++;
			} else {
				flat_inputs[flat_input_counter] = 0.0;
				flat_input_counter++;
			}
		}

		// dud
		flat_inputs[flat_input_counter] = rand()%2;
		flat_input_counter++;

		int total_sum = 0;
		bool first_block_is_even = (first_block_sum%2 == 0);
		if (first_block_is_even) {
			total_sum += 7;
		}
		bool second_block_is_even = (second_block_sum%2 == 0);
		if (second_block_is_even) {
			total_sum += 5;
		}

		vector<double> obs;
		obs.push_back(rand()%2);

		fold_network->activate(flat_inputs,
							   activated,
							   obs);

		average_error += abs(total_sum - fold_network->output->acti_vals[0]);
	}

	average_error /= 10000;
	cout << "average_error: " << average_error << endl;

	{
		delete fold_network;
		ifstream input_file;
		input_file.open("saves/f_8.txt");
		fold_network = new FoldNetwork(11, 1, input_file);
		input_file.close();
	}

	{
		state_network_num_inputs.push_back(0);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(1);
		state_network_num_outputs.push_back(1);

		ifstream input_file;
		input_file.open("saves/s_8_0.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}
	{
		state_network_num_inputs.push_back(1);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(0);
		state_network_num_outputs.push_back(1);

		ifstream input_file;
		input_file.open("saves/s_8_1.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}
	{
		state_network_num_inputs.push_back(0);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(0);
		state_network_num_outputs.push_back(0);

		ifstream input_file;
		input_file.open("saves/s_8_2.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}
	{
		state_network_num_inputs.push_back(1);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(0);
		state_network_num_outputs.push_back(1);

		ifstream input_file;
		input_file.open("saves/s_8_3.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}
	{
		state_network_num_inputs.push_back(1);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(0);
		state_network_num_outputs.push_back(1);

		ifstream input_file;
		input_file.open("saves/s_8_4.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}
	{
		state_network_num_inputs.push_back(0);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(0);
		state_network_num_outputs.push_back(0);

		ifstream input_file;
		input_file.open("saves/s_8_5.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}
	{
		state_network_num_inputs.push_back(0);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(1);
		state_network_num_outputs.push_back(1);

		ifstream input_file;
		input_file.open("saves/s_8_6.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}
	{
		state_network_num_inputs.push_back(1);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(0);
		state_network_num_outputs.push_back(1);

		ifstream input_file;
		input_file.open("saves/s_8_7.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}
	{
		state_network_num_inputs.push_back(1);
		state_network_num_compression.push_back(0);
		state_network_num_outputs_added.push_back(0);
		state_network_num_outputs.push_back(1);

		ifstream input_file;
		input_file.open("saves/s_8_8.txt");
		state_networks.push_back(new Network(input_file));
		input_file.close();

		fold_index++;
	}

	// int curr_input = 0;
	int curr_input = 2;
	for (int compress_index = 10; compress_index < 11; compress_index++) {
		int num_input = 2;
		int num_compression = 1;
		int num_output_added = 0;
		int num_output = 1;

		cout << "compress_index: " << compress_index << endl;
		cout << "num_input: " << num_input << endl;
		cout << "num_compression: " << num_compression << endl;
		cout << "num_output_added: " << num_output_added << endl;
		cout << "num_output: " << num_output << endl;

		while (true) {
			Network* network = new Network(num_input+1, 4*(num_input+1+num_output), num_output);
			FoldNetwork* copy_fold = new FoldNetwork(fold_network);
			int num_final_state = curr_input + num_output_added - num_compression;
			copy_fold->set_next_state_size(num_final_state);

			double sum_error = 0.0;
			for (int epoch_index = 1; epoch_index < 5000; epoch_index++) {
				if (epoch_index%100 == 0) {
					cout << endl;
					cout << epoch_index << endl;
					cout << "sum_error: " << sum_error << endl;
					sum_error = 0.0;
				}

				for (int iter_index = 0; iter_index < 100; iter_index++) {
					vector<double> state_vals;
					vector<double> next_state_vals;

					double flat_inputs[11];
					int flat_input_counter = 0;
					bool activated[11];
					for (int i = 0; i < 11; i++) {
						activated[i] = true;
					}

					// first block
					int first_block_sum = 0;
					for (int i = 0; i < 2; i++) {
						if (rand()%2 == 0) {
							flat_inputs[flat_input_counter] = 1.0;
							if (fold_index > flat_input_counter) {
								run_step(state_vals, 1.0, flat_input_counter);
							} else if (fold_index == flat_input_counter) { 
								run_test_network(state_vals,
												 next_state_vals,
												 1.0,
												 network,
												 num_input,
												 num_compression,
												 num_output_added,
												 num_output);
							}
							flat_input_counter++;
							first_block_sum++;
						} else {
							flat_inputs[flat_input_counter] = 0.0;
							if (fold_index > flat_input_counter) {
								run_step(state_vals, 0.0, flat_input_counter);
							} else if (fold_index == flat_input_counter) {
								run_test_network(state_vals,
												 next_state_vals,
												 0.0,
												 network,
												 num_input,
												 num_compression,
												 num_output_added,
												 num_output);
							}
							flat_input_counter++;
						}
					}

					// dud
					flat_inputs[flat_input_counter] = rand()%2;
					if (fold_index > flat_input_counter) {
						run_step(state_vals, rand()%2, flat_input_counter);
					} else if (fold_index == flat_input_counter) {
						run_test_network(state_vals,
										 next_state_vals,
										 rand()%2,
										 network,
										 num_input,
										 num_compression,
										 num_output_added,
										 num_output);
					}
					flat_input_counter++;

					for (int i = 0; i < 2; i++) {
						if (rand()%2 == 0) {
							flat_inputs[flat_input_counter] = 1.0;
							if (fold_index > flat_input_counter) {
								run_step(state_vals, 1.0, flat_input_counter);
							} else if (fold_index == flat_input_counter) { 
								run_test_network(state_vals,
												 next_state_vals,
												 1.0,
												 network,
												 num_input,
												 num_compression,
												 num_output_added,
												 num_output);
							}
							flat_input_counter++;
							first_block_sum++;
						} else {
							flat_inputs[flat_input_counter] = 0.0;
							if (fold_index > flat_input_counter) {
								run_step(state_vals, 0.0, flat_input_counter);
							} else if (fold_index == flat_input_counter) {
								run_test_network(state_vals,
												 next_state_vals,
												 0.0,
												 network,
												 num_input,
												 num_compression,
												 num_output_added,
												 num_output);
							}
							flat_input_counter++;
						}
					}

					// dud
					flat_inputs[flat_input_counter] = rand()%2;
					if (fold_index > flat_input_counter) {
						run_step(state_vals, rand()%2, flat_input_counter);
					} else if (fold_index == flat_input_counter) {
						run_test_network(state_vals,
										 next_state_vals,
										 rand()%2,
										 network,
										 num_input,
										 num_compression,
										 num_output_added,
										 num_output);
					}
					flat_input_counter++;

					// second block
					int second_block_sum = 0;
					for (int i = 0; i < 4; i++) {
						if (rand()%2 == 0) {
							flat_inputs[flat_input_counter] = 1.0;
							if (fold_index > flat_input_counter) {
								run_step(state_vals, 1.0, flat_input_counter);
							} else if (fold_index == flat_input_counter) { 
								run_test_network(state_vals,
												 next_state_vals,
												 1.0,
												 network,
												 num_input,
												 num_compression,
												 num_output_added,
												 num_output);
							}
							flat_input_counter++;
							second_block_sum++;
						} else {
							flat_inputs[flat_input_counter] = 0.0;
							if (fold_index > flat_input_counter) {
								run_step(state_vals, 0.0, flat_input_counter);
							} else if (fold_index == flat_input_counter) {
								run_test_network(state_vals,
												 next_state_vals,
												 0.0,
												 network,
												 num_input,
												 num_compression,
												 num_output_added,
												 num_output);
							}
							flat_input_counter++;
						}
					}

					// dud
					flat_inputs[flat_input_counter] = rand()%2;
					if (fold_index > flat_input_counter) {
						run_step(state_vals, rand()%2, flat_input_counter);
					} else if (fold_index == flat_input_counter) {
						run_test_network(state_vals,
										 next_state_vals,
										 rand()%2,
										 network,
										 num_input,
										 num_compression,
										 num_output_added,
										 num_output);
					}
					flat_input_counter++;

					int total_sum = 0;
					bool first_block_is_even = (first_block_sum%2 == 0);
					if (first_block_is_even) {
						total_sum += 7;
					}
					bool second_block_is_even = (second_block_sum%2 == 0);
					if (second_block_is_even) {
						total_sum += 5;
					}

					vector<double> obs;
					obs.push_back(rand()%2);

					// copy_fold->activate_curr(flat_inputs,
					// 						 activated,
					// 						 fold_index,
					// 						 obs,
					// 						 state_vals);
					// sum_error += abs(total_sum - copy_fold->output->acti_vals[0]);

					if (epoch_index < 2000) {
						copy_fold->activate_compare_hidden_and_backprop(
							flat_inputs,
							activated,
							fold_index,
							obs,
							state_vals,
							next_state_vals);

						vector<double> state_errors;
						for (int i = 0; i < num_final_state; i++) {
							state_errors.push_back(copy_fold->next_state_input->errors[i]);
							copy_fold->next_state_input->errors[i] = 0.0;
						}

						backprop_test_network(state_errors,
											  network,
											  num_input,
											  num_compression,
											  num_output_added,
											  num_output);
					} else {
						copy_fold->activate_full(flat_inputs,
												 activated,
												 fold_index,
												 obs,
												 next_state_vals);

						vector<double> errors;
						errors.push_back(total_sum - copy_fold->output->acti_vals[0]);
						sum_error += abs(errors[0]);

						copy_fold->backprop(errors);

						vector<double> state_errors;
						for (int i = 0; i < num_final_state; i++) {
							state_errors.push_back(copy_fold->next_state_input->errors[i]);
							copy_fold->next_state_input->errors[i] = 0.0;
						}

						backprop_test_network(state_errors,
											  network,
											  num_input,
											  num_compression,
											  num_output_added,
											  num_output);

						for (int i = fold_index - 1; i >= 0; i--) {
							backprop_step(state_errors, i);
						}
					}
				}
			}

			double new_average_error = 0.0;
			for (int iter_index = 0; iter_index < 10000; iter_index++) {
				vector<double> state_vals;
				vector<double> next_state_vals;

				double flat_inputs[11];
				int flat_input_counter = 0;
				bool activated[11];
				for (int i = 0; i < 11; i++) {
					activated[i] = true;
				}

				// first block
				int first_block_sum = 0;
				for (int i = 0; i < 2; i++) {
					if (rand()%2 == 0) {
						flat_inputs[flat_input_counter] = 1.0;
						if (fold_index > flat_input_counter) {
							run_step(state_vals, 1.0, flat_input_counter);
						} else if (fold_index == flat_input_counter) { 
							run_test_network(state_vals,
											 next_state_vals,
											 1.0,
											 network,
											 num_input,
											 num_compression,
											 num_output_added,
											 num_output);
						}
						flat_input_counter++;
						first_block_sum++;
					} else {
						flat_inputs[flat_input_counter] = 0.0;
						if (fold_index > flat_input_counter) {
							run_step(state_vals, 0.0, flat_input_counter);
						} else if (fold_index == flat_input_counter) {
							run_test_network(state_vals,
											 next_state_vals,
											 0.0,
											 network,
											 num_input,
											 num_compression,
											 num_output_added,
											 num_output);
						}
						flat_input_counter++;
					}
				}

				// dud
				flat_inputs[flat_input_counter] = rand()%2;
				if (fold_index > flat_input_counter) {
					run_step(state_vals, rand()%2, flat_input_counter);
				} else if (fold_index == flat_input_counter) {
					run_test_network(state_vals,
									 next_state_vals,
									 rand()%2,
									 network,
									 num_input,
									 num_compression,
									 num_output_added,
									 num_output);
				}
				flat_input_counter++;

				for (int i = 0; i < 2; i++) {
					if (rand()%2 == 0) {
						flat_inputs[flat_input_counter] = 1.0;
						if (fold_index > flat_input_counter) {
							run_step(state_vals, 1.0, flat_input_counter);
						} else if (fold_index == flat_input_counter) { 
							run_test_network(state_vals,
											 next_state_vals,
											 1.0,
											 network,
											 num_input,
											 num_compression,
											 num_output_added,
											 num_output);
						}
						flat_input_counter++;
						first_block_sum++;
					} else {
						flat_inputs[flat_input_counter] = 0.0;
						if (fold_index > flat_input_counter) {
							run_step(state_vals, 0.0, flat_input_counter);
						} else if (fold_index == flat_input_counter) {
							run_test_network(state_vals,
											 next_state_vals,
											 0.0,
											 network,
											 num_input,
											 num_compression,
											 num_output_added,
											 num_output);
						}
						flat_input_counter++;
					}
				}

				// dud
				flat_inputs[flat_input_counter] = rand()%2;
				if (fold_index > flat_input_counter) {
					run_step(state_vals, rand()%2, flat_input_counter);
				} else if (fold_index == flat_input_counter) {
					run_test_network(state_vals,
									 next_state_vals,
									 rand()%2,
									 network,
									 num_input,
									 num_compression,
									 num_output_added,
									 num_output);
				}
				flat_input_counter++;

				// second block
				int second_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					if (rand()%2 == 0) {
						flat_inputs[flat_input_counter] = 1.0;
						if (fold_index > flat_input_counter) {
							run_step(state_vals, 1.0, flat_input_counter);
						} else if (fold_index == flat_input_counter) { 
							run_test_network(state_vals,
											 next_state_vals,
											 1.0,
											 network,
											 num_input,
											 num_compression,
											 num_output_added,
											 num_output);
						}
						flat_input_counter++;
						second_block_sum++;
					} else {
						flat_inputs[flat_input_counter] = 0.0;
						if (fold_index > flat_input_counter) {
							run_step(state_vals, 0.0, flat_input_counter);
						} else if (fold_index == flat_input_counter) {
							run_test_network(state_vals,
											 next_state_vals,
											 0.0,
											 network,
											 num_input,
											 num_compression,
											 num_output_added,
											 num_output);
						}
						flat_input_counter++;
					}
				}

				// dud
				flat_inputs[flat_input_counter] = rand()%2;
				if (fold_index > flat_input_counter) {
					run_step(state_vals, rand()%2, flat_input_counter);
				} else if (fold_index == flat_input_counter) {
					run_test_network(state_vals,
									 next_state_vals,
									 rand()%2,
									 network,
									 num_input,
									 num_compression,
									 num_output_added,
									 num_output);
				}
				flat_input_counter++;

				int total_sum = 0;
				bool first_block_is_even = (first_block_sum%2 == 0);
				if (first_block_is_even) {
					total_sum += 7;
				}
				bool second_block_is_even = (second_block_sum%2 == 0);
				if (second_block_is_even) {
					total_sum += 5;
				}

				vector<double> obs;
				obs.push_back(rand()%2);

				copy_fold->activate_full(flat_inputs,
										 activated,
										 fold_index,
										 obs,
										 next_state_vals);

				new_average_error += abs(total_sum - copy_fold->output->acti_vals[0]);
			}

			new_average_error /= 10000;
			cout << "new_average_error: " << new_average_error << endl;

			if (new_average_error < 2.0*average_error) {
				copy_fold->take_next();

				state_network_num_inputs.push_back(num_input);
				state_network_num_compression.push_back(num_compression);
				state_network_num_outputs_added.push_back(num_output_added);
				state_network_num_outputs.push_back(num_output);
				state_networks.push_back(network);

				delete fold_network;
				fold_network = copy_fold;

				for (int s_index = 0; s_index < (int)state_networks.size(); s_index++) {
					ofstream output_file;
					output_file.open("saves/s_" + to_string(fold_index) + "_" + to_string(s_index) + ".txt");
					state_networks[s_index]->save(output_file);
					output_file.close();
				}

				ofstream output_file;
				output_file.open("saves/f_" + to_string(fold_index) + ".txt");
				fold_network->save(output_file);
				output_file.close();

				curr_input += num_output_added;

				fold_index++;
				break;
			} else {
				if (num_output >= curr_input) {
					num_output_added++;
					num_output++;
				} else {
					num_input++;
					num_output++;
				}

				delete network;
				delete copy_fold;
			}
		}
	}

	cout << "Done" << endl;
}
