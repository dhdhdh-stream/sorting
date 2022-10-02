#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_combine_network.h"
#include "fold_loop_init_network.h"
#include "fold_loop_network.h"
#include "fold_network.h"
#include "node.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// int loop_state_size = 2;
	int loop_state_size = 3;

	// vector<int> pre_loop_flat_sizes{1};
	// vector<int> loop_flat_sizes{1, 1, 1};
	// FoldLoopInitNetwork* init_network = new FoldLoopInitNetwork(pre_loop_flat_sizes,
	// 															loop_state_size);
	// FoldLoopNetwork* loop_network = new FoldLoopNetwork(loop_state_size,
	// 													pre_loop_flat_sizes,
	// 													loop_flat_sizes);

	// vector<int> post_loop_flat_sizes{1};
	// FoldCombineNetwork* combine_network = new FoldCombineNetwork(
	// 	loop_state_size,
	// 	pre_loop_flat_sizes,
	// 	post_loop_flat_sizes);

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
	// 	if (epoch_index%100 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		vector<AbstractNetworkHistory*> network_historys;

	// 		int num_iters = rand()%6;

	// 		int rand_non_empty = rand()%6;

	// 		vector<vector<double>> pre_loop_flat_vals{vector<double>{(double)rand_non_empty}};

	// 		init_network->activate(pre_loop_flat_vals);

	// 		vector<double> loop_state(loop_state_size);
	// 		for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 			loop_state[s_index] = init_network->output->acti_vals[s_index];
	// 		}

	// 		int add_third_value = -1+rand()%2*2;

	// 		int sum = 0;
	// 		for (int i = 0; i < 5; i++) {
	// 			int first_value = rand()%2;
	// 			int second_value = rand()%2;
	// 			int third_value = rand()%2;

	// 			if (i < rand_non_empty) {
	// 				if (first_value == second_value) {
	// 					sum += 2;
	// 				}
	// 			} else {
	// 				if (i < num_iters) {
	// 					sum -= 1;
	// 				}
	// 			}

	// 			if (i < num_iters) {
	// 				if (add_third_value == 1) {
	// 					sum += third_value;
	// 				}

	// 				vector<vector<double>> loop_flat_vals{
	// 					vector<double>{(double)first_value},
	// 					vector<double>{(double)second_value},
	// 					vector<double>{(double)third_value}};
	// 				loop_network->activate(loop_state,
	// 									   pre_loop_flat_vals,
	// 									   loop_flat_vals,
	// 									   network_historys);

	// 				for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 					loop_state[s_index] = loop_network->output->acti_vals[s_index];
	// 				}
	// 			}
	// 		}

	// 		vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
	// 		combine_network->activate(loop_state,
	// 								  pre_loop_flat_vals,
	// 								  post_loop_flat_vals);

	// 		vector<double> errors;
	// 		errors.push_back(sum - combine_network->output->acti_vals[0]);
	// 		sum_error += abs(errors[0]);

	// 		if (epoch_index < 40000) {
	// 			combine_network->backprop(errors, 0.01);
	// 		} else {
	// 			combine_network->backprop(errors, 0.001);
	// 		}

	// 		vector<double> loop_state_errors;
	// 		for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 			loop_state_errors.push_back(combine_network->loop_state_input->errors[s_index]);
	// 			combine_network->loop_state_input->errors[s_index] = 0.0;
	// 		}

	// 		while (network_historys.size() > 0) {
	// 			network_historys.back()->reset_weights();

	// 			if (epoch_index < 40000) {
	// 				loop_network->backprop(loop_state_errors,
	// 									   0.01);
	// 			} else {
	// 				loop_network->backprop(loop_state_errors,
	// 									   0.001);
	// 			}
	// 			for (int s_index = 0; s_index < loop_state_size; s_index++) {
	// 				loop_state_errors[s_index] = loop_network->loop_state_input->errors[s_index];
	// 				loop_network->loop_state_input->errors[s_index] = 0.0;
	// 			}

	// 			delete network_historys.back();
	// 			network_historys.pop_back();
	// 		}

	// 		if (epoch_index < 40000) {
	// 			init_network->backprop(loop_state_errors,
	// 								   0.01);
	// 		} else {
	// 			init_network->backprop(loop_state_errors,
	// 								   0.001);
	// 		}
	// 	}
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/init_network.txt");
	// 	init_network->save(output_file);
	// 	output_file.close();
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/loop_network.txt");
	// 	loop_network->save(output_file);
	// 	output_file.close();
	// }

	// {
	// 	ofstream output_file;
	// 	output_file.open("saves/combine_network.txt");
	// 	combine_network->save(output_file);
	// 	output_file.close();
	// }

	ifstream init_input_file;
	init_input_file.open("saves/init_network.txt");
	FoldLoopInitNetwork* init_network = new FoldLoopInitNetwork(init_input_file);
	init_input_file.close();
	
	ifstream loop_input_file;
	loop_input_file.open("saves/loop_network.txt");
	FoldLoopNetwork* loop_network = new FoldLoopNetwork(loop_input_file);
	loop_input_file.close();

	ifstream combine_input_file;
	combine_input_file.open("saves/combine_network.txt");
	FoldCombineNetwork* combine_network = new FoldCombineNetwork(combine_input_file);
	combine_input_file.close();

	double average_error = 0.0;
	for (int iter_index = 0; iter_index < 10000; iter_index++) {
	// for (int iter_index = 0; iter_index < 1; iter_index++) {
		int num_iters = rand()%6;
		// cout << "num_iters: " << num_iters << endl;

		int rand_non_empty = rand()%6;
		// cout << "rand_non_empty: " << rand_non_empty << endl;

		vector<vector<double>> pre_loop_flat_vals{vector<double>{(double)rand_non_empty}};

		init_network->activate(pre_loop_flat_vals);

		vector<double> loop_state(loop_state_size);
		// cout << "start:";
		for (int s_index = 0; s_index < loop_state_size; s_index++) {
			loop_state[s_index] = init_network->output->acti_vals[s_index];
			// cout << " " << loop_state[s_index];
		}
		// cout << endl;

		int add_third_value = -1+rand()%2*2;
		// cout << "add_third_value: " << add_third_value << endl;

		int sum = 0;
		for (int i = 0; i < 5; i++) {
			int first_value = rand()%2;
			// cout << i << " first_value: " << first_value << endl;
			int second_value = rand()%2;
			// cout << i << " second_value: " << second_value << endl;
			int third_value = rand()%2;
			// cout << i << " third_value: " << third_value << endl;

			if (i < rand_non_empty) {
				if (first_value == second_value) {
					sum += 2;
				}
			} else {
				if (i < num_iters) {
					sum -= 1;
				}
			}

			if (i < num_iters) {
				if (add_third_value == 1) {
					sum += third_value;
				}

				vector<vector<double>> loop_flat_vals{
					vector<double>{(double)first_value},
					vector<double>{(double)second_value},
					vector<double>{(double)third_value}
				};
				loop_network->activate(loop_state,
									   pre_loop_flat_vals,
									   loop_flat_vals);
				// cout << i << ":";
				for (int s_index = 0; s_index < loop_state_size; s_index++) {
					loop_state[s_index] = loop_network->output->acti_vals[s_index];
					// cout << " " << loop_state[s_index];
				}
				// cout << endl;
			}
		}

		vector<vector<double>> post_loop_flat_vals{vector<double>{(double)add_third_value}};
		combine_network->activate(loop_state,
								  pre_loop_flat_vals,
								  post_loop_flat_vals);

		average_error += abs(sum - combine_network->output->acti_vals[0]);

		// cout << "target: " << sum << endl;
		// cout << "actual: " << combine_network->output->acti_vals[0] << endl;
	}

	average_error /= 10000;
	cout << "average_error: " << average_error << endl;

	delete init_network;
	delete loop_network;
	delete combine_network;

	cout << "Done" << endl;
}
