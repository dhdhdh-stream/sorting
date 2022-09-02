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

	int state_size = 2;

	Node* start_node = new Node(state_size);
	Node* posi_node = new Node(state_size);
	Node* negi_node = new Node(state_size);
	Node* end_node = new Node(state_size);

	vector<Node*> input_mappings;
	input_mappings.push_back(start_node);
	for (int i = 0; i < 6; i++) {
		input_mappings.push_back(posi_node);
		input_mappings.push_back(negi_node);
	}
	input_mappings.push_back(end_node);

	FoldNetwork fold_network(input_mappings, state_size);

	double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 20000; epoch_index++) {
	for (int epoch_index = 1; epoch_index < 50000; epoch_index++) {
		if (epoch_index%1000 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			double state_vals[state_size] = {};

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

			double state_errors[state_size] = {};

			fold_network.full_backprop(errors,
									   state_errors);
		}
	}

	// ofstream output_file;
	// output_file.open("saves/nns/fold_network.txt");
	// fold_network.save(output_file);
	// output_file.close();

	// ifstream input_file;
	// input_file.open("saves/nns/fold_network.txt");
	// FoldNetwork fold_network(input_mappings, state_size, input_file);
	// input_file.close();

	for (int i_index = 0; i_index < (int)input_mappings.size(); i_index++) {
		cout << "folding " << i_index << endl;
		
		fold_network.input_on[i_index] = false;

		double sum_error = 0.0;
		// for (int epoch_index = 1; epoch_index < 4000; epoch_index++) {
		for (int epoch_index = 1; epoch_index < 20000; epoch_index++) {
			if (epoch_index%1000 == 0) {
				cout << endl;
				cout << epoch_index << endl;
				cout << "sum_error: " << sum_error << endl;
				sum_error = 0.0;
			}

			for (int iter_index = 0; iter_index < 100; iter_index++) {
				double state_vals[state_size] = {};

				int sum = 0;

				vector<Node*> nodes_visited;
				vector<NetworkHistory*> network_historys;

				double start_obs;
				if (rand()%2 == 0) {
					start_obs = 1.0;
					sum++;
				} else {
					start_obs = 0.0;
				}
				// start_node->activate(start_obs,
				// 					 state_vals,
				// 					 network_historys);
				start_node->activate_greedy(start_obs,
											state_vals,
											network_historys);
				nodes_visited.push_back(start_node);

				vector<double> inputs;
				vector<bool> activated;

				inputs.push_back(0.0);
				activated.push_back(true);

				for (int i = 0; i < 6; i++) {
					bool is_posi = (rand()%2 == 0);
					bool rand_on = (rand()%2 == 0);

					if (is_posi) {
						if (!fold_network.input_on[1+2*i]) {
							if (rand_on) {
								// posi_node->activate(1.0,
								// 					state_vals,
								// 					network_historys);
								posi_node->activate_greedy(1.0,
														   state_vals,
														   network_historys);
							} else {
								// posi_node->activate(0.0,
								// 					state_vals,
								// 					network_historys);
								posi_node->activate_greedy(0.0,
														   state_vals,
														   network_historys);
							}
							nodes_visited.push_back(posi_node);
						}

						if (rand_on) {
							inputs.push_back(1.0);
							sum++;
						} else {
							inputs.push_back(0.0);
						}
						inputs.push_back(0.0);

						activated.push_back(true);
						activated.push_back(false);
					} else {
						if (!fold_network.input_on[1+2*i+1]) {
							if (rand_on) {
								// negi_node->activate(-1.0,
								// 					state_vals,
								// 					network_historys);
								negi_node->activate_greedy(-1.0,
														   state_vals,
														   network_historys);
							} else {
								// negi_node->activate(0.0,
								// 					state_vals,
								// 					network_historys);
								negi_node->activate_greedy(0.0,
														   state_vals,
														   network_historys);
							}
							nodes_visited.push_back(negi_node);
						}

						inputs.push_back(0.0);
						if (rand_on) {
							inputs.push_back(-1.0);
							sum++;
						} else {
							inputs.push_back(0.0);
						}

						activated.push_back(false);
						activated.push_back(true);
					}
				}

				bool rand_on = (rand()%2 == 0);
				if (!fold_network.input_on[13]) {
					if (rand_on) {
						// end_node->activate(1.0,
						// 				   state_vals,
						// 				   network_historys);
						end_node->activate_greedy(1.0,
												  state_vals,
												  network_historys);
					} else {
						// end_node->activate(0.0,
						// 				   state_vals,
						// 				   network_historys);
						end_node->activate_greedy(0.0,
												  state_vals,
												  network_historys);
					}
					nodes_visited.push_back(end_node);
				}
				if (rand_on) {
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

				double state_errors[state_size] = {};

				if (epoch_index < 5000) {
					fold_network.state_backprop(errors,
												state_errors);
				} else {
					fold_network.full_backprop(errors,
											   state_errors);
				}

				for (int n_index = (int)nodes_visited.size()-1; n_index >= 0; n_index--) {
					// nodes_visited[n_index]->backprop(state_errors,
					// 								 network_historys);
					nodes_visited[n_index]->backprop_greedy(state_errors,
															network_historys);
				}
			}
		}
	}

	// ofstream start_save_file;
	// start_save_file.open("saves/nns/start_equals.txt");
	// start_node->network->save(start_save_file);
	// start_save_file.close();

	// ofstream posi_save_file;
	// posi_save_file.open("saves/nns/posi_equals.txt");
	// posi_node->network->save(posi_save_file);
	// posi_save_file.close();

	// ofstream negi_save_file;
	// negi_save_file.open("saves/nns/negi_equals.txt");
	// negi_node->network->save(negi_save_file);
	// negi_save_file.close();

	// ofstream end_save_file;
	// end_save_file.open("saves/nns/end_equals.txt");
	// end_node->network->save(end_save_file);
	// end_save_file.close();

	ofstream start_save_file;
	start_save_file.open("saves/nns/start_greedy.txt");
	start_node->network->save(start_save_file);
	start_save_file.close();

	ofstream posi_save_file;
	posi_save_file.open("saves/nns/posi_greedy.txt");
	posi_node->network->save(posi_save_file);
	posi_save_file.close();

	ofstream negi_save_file;
	negi_save_file.open("saves/nns/negi_greedy.txt");
	negi_node->network->save(negi_save_file);
	negi_save_file.close();

	ofstream end_save_file;
	end_save_file.open("saves/nns/end_greedy.txt");
	end_node->network->save(end_save_file);
	end_save_file.close();

	cout << "Done" << endl;
}
