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

	delete start_node->network;
	ifstream start_save_file;
	start_save_file.open("saves/nns/start_greedy.txt");
	start_node->network = new Network(start_save_file);
	start_save_file.close();

	delete posi_node->network;
	ifstream posi_save_file;
	posi_save_file.open("saves/nns/posi_greedy.txt");
	posi_node->network = new Network(posi_save_file);
	posi_save_file.close();

	delete negi_node->network;
	ifstream negi_save_file;
	negi_save_file.open("saves/nns/negi_greedy.txt");
	negi_node->network = new Network(negi_save_file);
	negi_save_file.close();

	delete end_node->network;
	ifstream end_save_file;
	end_save_file.open("saves/nns/end_greedy.txt");
	end_node->network = new Network(end_save_file);
	end_save_file.close();

	Network score_network(2, 32, 1);

	double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 5000; epoch_index++) {
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

			double start_obs;
			if (rand()%2 == 0) {
				start_obs = 1.0;
				sum++;
			} else {
				start_obs = 0.0;
			}
			// start_node->activate(start_obs,
			// 					 state_vals);
			start_node->activate_greedy(start_obs,
										state_vals);
			// cout << "start " << start_obs << ": " << state_vals[0] << " " << state_vals[1] << endl;

			for (int i = 0; i < 6; i++) {
				bool is_posi = (rand()%2 == 0);
				bool rand_on = (rand()%2 == 0);

				if (is_posi) {
					if (rand_on) {
						// posi_node->activate(1.0,
						// 					state_vals);
						posi_node->activate_greedy(1.0,
												   state_vals);
						// cout << i << " posi 1.0: " << state_vals[0] << " " << state_vals[1] << endl;
					} else {
						// posi_node->activate(0.0,
						// 					state_vals);
						posi_node->activate_greedy(0.0,
												   state_vals);
						// cout << i << " posi 0.0: " << state_vals[0] << " " << state_vals[1] << endl;
					}
				} else {
					if (rand_on) {
						// negi_node->activate(-1.0,
						// 					state_vals);
						negi_node->activate_greedy(-1.0,
												   state_vals);
						// cout << i << " negi -1.0: " << state_vals[0] << " " << state_vals[1] << endl;
					} else {
						// negi_node->activate(0.0,
						// 					state_vals);
						negi_node->activate_greedy(0.0,
												   state_vals);
						// cout << i << " negi 0.0: " << state_vals[0] << " " << state_vals[1] << endl;
					}
				}

				if (rand_on) {
					sum++;
				}
			}

			bool rand_on = (rand()%2 == 0);
			if (rand_on) {
				// end_node->activate(1.0,
				// 				   state_vals);
				end_node->activate_greedy(1.0,
										  state_vals);
				// cout << "end 1.0: " << state_vals[0] << " " << state_vals[1] << endl;
			} else {
				// end_node->activate(0.0,
				// 				   state_vals);
				end_node->activate_greedy(0.0,
										  state_vals);
				// cout << "end 0.0: " << state_vals[0] << " " << state_vals[1] << endl;
			}
			if (rand_on) {
				sum++;
			}

			bool is_even = (sum%2 == 0);

			vector<double> score_inputs;
			score_inputs.push_back(state_vals[0]);
			score_inputs.push_back(state_vals[1]);
			score_network.activate(score_inputs);

			vector<double> errors;
			if (is_even) {
				if (score_network.output->acti_vals[0] > 1.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(1.0 - score_network.output->acti_vals[0]);
				}
			} else {
				if (score_network.output->acti_vals[0] < 0.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(0.0 - score_network.output->acti_vals[0]);
				}
			}
			sum_error += abs(errors[0]);

			score_network.backprop(errors);
		}
	}

	Node* replace_node_1 = new Node(state_size);
	Node* replace_node_2 = new Node(state_size);
	// Node* replace_node_3 = new Node(state_size);

	sum_error = 0.0;
	// double sum_error = 0.0;
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

			vector<Node*> existing_nodes_visited;
			vector<NetworkHistory*> existing_network_historys;

			vector<NetworkHistory*> new_network_historys;

			double start_obs;
			if (rand()%2 == 0) {
				start_obs = 1.0;
				sum++;
			} else {
				start_obs = 0.0;
			}
			// start_node->activate(start_obs,
			// 					 state_vals);
			start_node->activate_greedy(start_obs,
										state_vals);

			double replace_1_obs;
			if (rand()%2 == 0) {
				replace_1_obs = 1.0;
				sum++;
			} else {
				replace_1_obs = 0.0;
			}
			replace_node_1->activate_greedy(replace_1_obs,
											state_vals,
											new_network_historys);
			// replace_node_1->activate(replace_1_obs,
			// 						 state_vals,
			// 						 new_network_historys);

			double replace_2_obs;
			if (rand()%2 == 0) {
				replace_2_obs = 1.0;
				sum++;
			} else {
				replace_2_obs = 0.0;
			}
			replace_node_2->activate_greedy(replace_2_obs,
											state_vals,
											new_network_historys);

			// double replace_3_obs;
			// if (rand()%2 == 0) {
			// 	replace_3_obs = 1.0;
			// 	sum++;
			// } else {
			// 	replace_3_obs = 0.0;
			// }
			// replace_node_3->activate_greedy(replace_3_obs,
			// 								state_vals,
			// 								new_network_historys);

			for (int i = 0; i < 4; i++) {
			// for (int i = 0; i < 3; i++) {
				bool is_posi = (rand()%2 == 0);
				bool rand_on = (rand()%2 == 0);

				if (is_posi) {
					if (rand_on) {
						// posi_node->activate(1.0,
						// 					state_vals,
						// 					existing_network_historys);
						posi_node->activate_greedy(1.0,
												   state_vals,
												   existing_network_historys);
					} else {
						// posi_node->activate(0.0,
						// 					state_vals,
						// 					existing_network_historys);
						posi_node->activate_greedy(0.0,
												   state_vals,
												   existing_network_historys);
					}
					existing_nodes_visited.push_back(posi_node);
				} else {
					if (rand_on) {
						// negi_node->activate(-1.0,
						// 					state_vals,
						// 					existing_network_historys);
						negi_node->activate_greedy(-1.0,
												   state_vals,
												   existing_network_historys);
					} else {
						// negi_node->activate(0.0,
						// 					state_vals,
						// 					existing_network_historys);
						negi_node->activate_greedy(0.0,
												   state_vals,
												   existing_network_historys);
					}
					existing_nodes_visited.push_back(negi_node);
				}

				if (rand_on) {
					sum++;
				}
			}

			bool rand_on = (rand()%2 == 0);
			if (rand_on) {
				// end_node->activate(1.0,
				// 				   state_vals,
				// 				   existing_network_historys);
				end_node->activate_greedy(1.0,
										  state_vals,
										  existing_network_historys);
			} else {
				// end_node->activate(0.0,
				// 				   state_vals,
				// 				   existing_network_historys);
				end_node->activate_greedy(0.0,
										  state_vals,
										  existing_network_historys);
			}
			existing_nodes_visited.push_back(end_node);
			if (rand_on) {
				sum++;
			}

			bool is_even = (sum%2 == 0);

			vector<double> score_inputs;
			score_inputs.push_back(state_vals[0]);
			score_inputs.push_back(state_vals[1]);
			score_network.activate(score_inputs);

			vector<double> errors;
			if (is_even) {
				if (score_network.output->acti_vals[0] > 1.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(1.0 - score_network.output->acti_vals[0]);
				}
			} else {
				if (score_network.output->acti_vals[0] < 0.0) {
					errors.push_back(0.0);
				} else {
					errors.push_back(0.0 - score_network.output->acti_vals[0]);
				}
			}
			sum_error += abs(errors[0]);

			double state_errors[state_size] = {};
			
			score_network.backprop_errors_with_no_weight_change(errors);

			state_errors[0] = score_network.input->errors[0];
			score_network.input->errors[0] = 0.0;
			state_errors[1] = score_network.input->errors[1];
			score_network.input->errors[1] = 0.0;

			for (int n_index = (int)existing_nodes_visited.size()-1; n_index >= 0; n_index--) {
				// existing_nodes_visited[n_index]->backprop_errors_with_no_weight_change(
				// 	state_errors,
				// 	existing_network_historys);
				existing_nodes_visited[n_index]->backprop_greedy_errors_with_no_weight_change(
					state_errors,
					existing_network_historys);
			}

			// replace_node_3->backprop_greedy(state_errors,
			// 								new_network_historys);
			replace_node_2->backprop_greedy(state_errors,
											new_network_historys);
			replace_node_1->backprop_greedy(state_errors,
											new_network_historys);
			// replace_node_1->backprop(state_errors,
			// 						 new_network_historys);
		}
	}

	// sum_error = 0.0;
	// // double sum_error = 0.0;
	// for (int epoch_index = 1; epoch_index < 5000; epoch_index++) {
	// 	if (epoch_index%1000 == 0) {
	// 		cout << endl;
	// 		cout << epoch_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	for (int iter_index = 0; iter_index < 100; iter_index++) {
	// 		double state_vals[state_size] = {};

	// 		int sum = 0;

	// 		double start_obs;
	// 		if (rand()%2 == 0) {
	// 			start_obs = 1.0;
	// 			sum++;
	// 		} else {
	// 			start_obs = 0.0;
	// 		}
	// 		start_node->activate(start_obs,
	// 							 state_vals);

	// 		for (int i = 0; i < 6; i++) {
	// 			bool is_posi = (rand()%2 == 0);
	// 			bool rand_on = (rand()%2 == 0);

	// 			if (is_posi) {
	// 				if (rand_on) {
	// 					posi_node->activate(1.0,
	// 										state_vals);
	// 				} else {
	// 					posi_node->activate(0.0,
	// 										state_vals);
	// 				}
	// 			} else {
	// 				if (rand_on) {
	// 					negi_node->activate(-1.0,
	// 										state_vals);
	// 				} else {
	// 					negi_node->activate(0.0,
	// 										state_vals);
	// 				}
	// 			}

	// 			if (rand_on) {
	// 				sum++;
	// 			}
	// 		}

	// 		bool rand_on = (rand()%2 == 0);
	// 		if (rand_on) {
	// 			end_node->activate(1.0,
	// 							   state_vals);
	// 		} else {
	// 			end_node->activate(0.0,
	// 							   state_vals);
	// 		}
	// 		if (rand_on) {
	// 			sum++;
	// 		}

	// 		bool is_even = (sum%2 == 0);

	// 		vector<double> score_inputs;
	// 		score_inputs.push_back(state_vals[0]);
	// 		score_inputs.push_back(state_vals[1]);
	// 		score_network.activate(score_inputs);

	// 		vector<double> errors;
	// 		if (is_even) {
	// 			if (score_network.output->acti_vals[0] > 1.0) {
	// 				errors.push_back(0.0);
	// 			} else {
	// 				errors.push_back(1.0 - score_network.output->acti_vals[0]);
	// 			}
	// 		} else {
	// 			if (score_network.output->acti_vals[0] < 0.0) {
	// 				errors.push_back(0.0);
	// 			} else {
	// 				errors.push_back(0.0 - score_network.output->acti_vals[0]);
	// 			}
	// 		}
	// 		sum_error += abs(errors[0]);

	// 		score_network.backprop(errors);
	// 	}
	// }

	cout << "Done" << endl;
}
