#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	Node* start_node = new Node(2);
	Node* posi_node = new Node(2);
	Node* negi_node = new Node(2);
	Node* end_node = new Node(2);

	vector<Node*> input_mappings;
	input_mappings.push_back(start_node);
	for (int i = 0; i < 6; i++) {
		input_mappings.push_back(posi_node);
		input_mappings.push_back(negi_node);
	}
	input_mappings.push_back(end_node);

	// FoldNetwork fold_network(input_mappings, 2);
	
	ifstream input_file;
	input_file.open("saves/nns/fold_network.txt");
	FoldNetwork fold_network(input_mappings, 2, input_file);
	input_file.close();

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
		if (epoch_index%1000 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			double state_vals[2] = {};

			int sum = 0;

			vector<double> inputs;
			vector<bool> activated;

			if (rand()%2 == 0) {
				inputs.push_back(1.0);
				sum++;
			} else {
				inputs.push_back(0.0);
			}
			activated.push_back(true);

			for (int i = 0; i < 6; i++) {
				bool is_posi = (rand()%2 == 0);
				if (is_posi) {
					if (rand()%2 == 0) {
						inputs.push_back(1.0);
						inputs.push_back(0.0);

						activated.push_back(true);
						activated.push_back(false);

						sum++;
					} else {
						inputs.push_back(0.0);
						inputs.push_back(0.0);

						activated.push_back(true);
						activated.push_back(false);
					}
				} else {
					if (rand()%2 == 0) {
						inputs.push_back(0.0);
						inputs.push_back(-1.0);

						activated.push_back(false);
						activated.push_back(true);

						sum++;
					} else {
						inputs.push_back(0.0);
						inputs.push_back(0.0);

						activated.push_back(false);
						activated.push_back(true);
					}
				}
			}

			if (rand()%2 == 0) {
				inputs.push_back(1.0);
				sum++;
			} else {
				inputs.push_back(0.0);
			}
			activated.push_back(true);

			bool is_even = (sum%2 == 0);

			fold_network.activate(inputs,
								  activated,
								  state_vals);

			vector<double> errors;
			if (is_even) {
				if (fold_network.output->acti_vals[0] > 1.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(1.0 - fold_network.output->acti_vals[0]);
				}
			} else {
				if (fold_network.output->acti_vals[0] < 0.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(0.0 - fold_network.output->acti_vals[0]);
				}
			}
			sum_error += abs(errors[0]);

			double state_errors[2] = {};

			fold_network.full_backprop(errors,
									   state_errors);
		}
	}

	// ofstream output_file;
	// output_file.open("saves/nns/fold_network.txt");
	// fold_network.save(output_file);
	// output_file.close();

	// start_node->network_on = true;

	// double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
	// 	if (epoch_index%1000 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		double state_vals[2] = {};

	// 		int sum = 0;

	// 		vector<NetworkHistory*> network_historys;

	// 		double start_obs;
	// 		if (rand()%2 == 0) {
	// 			start_obs = 1.0;
	// 			sum++;
	// 		} else {
	// 			start_obs = 0.0;
	// 		}
	// 		start_node->activate(start_obs,
	// 							 state_vals,
	// 							 network_historys);

	// 		vector<double> inputs;
	// 		vector<bool> activated;

	// 		inputs.push_back(0.0);
	// 		activated.push_back(true);

	// 		for (int i = 0; i < 6; i++) {
	// 			bool is_posi = (rand()%2 == 0);
	// 			if (is_posi) {
	// 				if (rand()%2 == 0) {
	// 					inputs.push_back(1.0);
	// 					inputs.push_back(0.0);

	// 					activated.push_back(true);
	// 					activated.push_back(false);

	// 					sum++;
	// 				} else {
	// 					inputs.push_back(0.0);
	// 					inputs.push_back(0.0);

	// 					activated.push_back(true);
	// 					activated.push_back(false);
	// 				}
	// 			} else {
	// 				if (rand()%2 == 0) {
	// 					inputs.push_back(0.0);
	// 					inputs.push_back(-1.0);

	// 					activated.push_back(false);
	// 					activated.push_back(true);

	// 					sum++;
	// 				} else {
	// 					inputs.push_back(0.0);
	// 					inputs.push_back(0.0);

	// 					activated.push_back(false);
	// 					activated.push_back(true);
	// 				}
	// 			}
	// 		}

	// 		if (rand()%2 == 0) {
	// 			inputs.push_back(1.0);
	// 			sum++;
	// 		} else {
	// 			inputs.push_back(0.0);
	// 		}
	// 		activated.push_back(true);

	// 		bool is_even = (sum%2 == 0);

	// 		fold_network.activate(inputs,
	// 							  activated,
	// 							  state_vals);

	// 		vector<double> errors;
	// 		if (is_even) {
	// 			if (fold_network.output->acti_vals[0] > 1.0) {
	// 				errors.push_back(0.0);
	// 			} else {
	// 				errors.push_back(1.0 - fold_network.output->acti_vals[0]);
	// 			}
	// 		} else {
	// 			if (fold_network.output->acti_vals[0] < 0.0) {
	// 				errors.push_back(0.0);
	// 			} else {
	// 				errors.push_back(0.0 - fold_network.output->acti_vals[0]);
	// 			}
	// 		}
	// 		sum_error += abs(errors[0]);

	// 		double state_errors[2] = {};

	// 		fold_network.full_backprop(errors,
	// 								   state_errors);

	// 		start_node->backprop(state_errors,
	// 							 network_historys);
	// 	}
	// }

	cout << "Done" << endl;
}
