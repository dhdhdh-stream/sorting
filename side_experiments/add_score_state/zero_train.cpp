#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

void zero_train_step(int input_counter,
					 int scope_start,
					 int scope_end,
					 double observation,
					 vector<vector<double>>& state_vals,
					 vector<bool>& scopes_on,
					 vector<vector<double>>& zero_state_vals,
					 vector<bool>& zero_scopes_on,
					 vector<Node*>& nodes,
					 vector<Node*>& zero_nodes,
					 double& sum_error) {
	if (input_counter < scope_start) {
		nodes[input_counter]->activate(state_vals,
									   scopes_on,
									   observation);
	} else if (input_counter == scope_start) {
		nodes[input_counter]->activate(state_vals,
									   scopes_on,
									   observation);
		zero_state_vals = state_vals;
		zero_scopes_on = scopes_on;
		for (int sc_index = 0; sc_index < (int)zero_state_vals.size()-1; sc_index++) {
			for (int st_index = 0; st_index < (int)zero_state_vals[sc_index].size(); st_index++) {
				zero_state_vals[sc_index][st_index] = 0.0;
			}
			zero_scopes_on[sc_index] = false;
		}
	} else if (input_counter < scope_end) {
		nodes[input_counter]->activate(state_vals,
									   scopes_on,
									   observation);
		zero_nodes[input_counter]->activate(zero_state_vals,
											zero_scopes_on,
											observation);
		zero_nodes[input_counter]->backprop_zero_train(nodes[input_counter],
													   sum_error);
	} else if (input_counter == scope_end) { 
		nodes[input_counter]->activate_state(state_vals,
											 scopes_on,
											 observation);
		zero_nodes[input_counter]->activate_state(zero_state_vals,
												  zero_scopes_on,
												  observation);
		zero_nodes[input_counter]->backprop_zero_train_state(nodes[input_counter],
															 sum_error);
	}
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Node*> nodes;
	for (int i = 0; i < 10; i++) {
		ifstream input_file;
		input_file.open("saves/r2_p2_n_" + to_string(i) + "_9.txt");
		nodes.push_back(new Node("r2_p2_n_"+to_string(i), input_file));
		input_file.close();
	}

	vector<Node*> zero_nodes;
	for (int i = 0; i < 10; i++) {
		zero_nodes.push_back(new Node(nodes[i]));
	}

	double average_score = 0.0;

	vector<int> scope_starts;
	vector<int> scope_ends;

	vector<int> process_scope_stack;
	vector<int> scope_sizes{1};
	for (int i = 0; i < 10; i++) {
		int starting_num_scopes = (int)scope_sizes.size();
		// TODO: handle multiple compression_networks
		nodes[i]->get_scope_sizes(scope_sizes);
		int ending_num_scopes = (int)scope_sizes.size();

		if (ending_num_scopes > starting_num_scopes) {
			process_scope_stack.push_back(i);
		} else if (ending_num_scopes < starting_num_scopes) {
			int num_diff = starting_num_scopes - ending_num_scopes;
			for (int d_index = 0; d_index < num_diff-1; d_index++) {
				process_scope_stack.pop_back();
			}
			scope_starts.push_back(process_scope_stack.back());
			scope_ends.push_back(i);
			process_scope_stack.pop_back();
		}
	}

	double sum_error = 0.0;
	for (int epoch_index = 1; epoch_index < 20000; epoch_index++) {
		if (epoch_index%100 == 0) {
			cout << endl;
			cout << epoch_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		for (int iter_index = 0; iter_index < 100; iter_index++) {
			int rand_scope_index = -1 + rand()%((int)scope_starts.size()+1);

			if (rand_scope_index == -1) {
				vector<vector<double>> state_vals;
				state_vals.push_back(vector<double>{average_score});
				vector<bool> scopes_on;
				scopes_on.push_back(true);

				vector<vector<double>> zero_state_vals = state_vals;
				vector<bool> zero_scopes_on = scopes_on;

				int flat_input_counter = 0;

				// first block
				int first_block_on_index = rand()%5;
				nodes[flat_input_counter]->activate(state_vals,
													scopes_on,
													first_block_on_index);
				zero_nodes[flat_input_counter]->activate(zero_state_vals,
														 zero_scopes_on,
														 first_block_on_index);
				zero_nodes[flat_input_counter]->backprop_zero_train(nodes[flat_input_counter],
																	sum_error);
				flat_input_counter++;

				int first_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					nodes[flat_input_counter]->activate(state_vals,
														scopes_on,
														value);
					zero_nodes[flat_input_counter]->activate(zero_state_vals,
															 zero_scopes_on,
															 value);
					zero_nodes[flat_input_counter]->backprop_zero_train(nodes[flat_input_counter],
																		sum_error);
					flat_input_counter++;
					if (i < first_block_on_index) {
						first_block_sum += value;
					}
				}

				// second block
				int second_block_on_index = rand()%5;
				nodes[flat_input_counter]->activate(state_vals,
													scopes_on,
													second_block_on_index);
				zero_nodes[flat_input_counter]->activate(zero_state_vals,
														 zero_scopes_on,
														 second_block_on_index);
				zero_nodes[flat_input_counter]->backprop_zero_train(nodes[flat_input_counter],
																	sum_error);
				flat_input_counter++;

				int second_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					nodes[flat_input_counter]->activate(state_vals,
														scopes_on,
														value);
					zero_nodes[flat_input_counter]->activate(zero_state_vals,
															 zero_scopes_on,
															 value);
					zero_nodes[flat_input_counter]->backprop_zero_train(nodes[flat_input_counter],
																		sum_error);
					flat_input_counter++;
					if (i < second_block_on_index) {
						second_block_sum += value;
					}
				}
			} else {
				vector<vector<double>> state_vals;
				state_vals.push_back(vector<double>{average_score});
				vector<bool> scopes_on;
				scopes_on.push_back(true);

				vector<vector<double>> zero_state_vals;
				vector<bool> zero_scopes_on;

				int flat_input_counter = 0;

				// first block
				int first_block_on_index = rand()%5;
				zero_train_step(flat_input_counter,
								scope_starts[rand_scope_index],
								scope_ends[rand_scope_index],
								first_block_on_index,
								state_vals,
								scopes_on,
								zero_state_vals,
								zero_scopes_on,
								nodes,
								zero_nodes,
								sum_error);
				if (flat_input_counter == scope_ends[rand_scope_index]) {
					continue;
				}
				flat_input_counter++;

				int first_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					zero_train_step(flat_input_counter,
									scope_starts[rand_scope_index],
									scope_ends[rand_scope_index],
									value,
									state_vals,
									scopes_on,
									zero_state_vals,
									zero_scopes_on,
									nodes,
									zero_nodes,
									sum_error);
					if (flat_input_counter == scope_ends[rand_scope_index]) {
						break;
					}
					flat_input_counter++;
					if (i < first_block_on_index) {
						first_block_sum += value;
					}
				}
				if (flat_input_counter == scope_ends[rand_scope_index]) {
					continue;
				}

				// second block
				int second_block_on_index = rand()%5;
				zero_train_step(flat_input_counter,
								scope_starts[rand_scope_index],
								scope_ends[rand_scope_index],
								second_block_on_index,
								state_vals,
								scopes_on,
								zero_state_vals,
								zero_scopes_on,
								nodes,
								zero_nodes,
								sum_error);
				if (flat_input_counter == scope_ends[rand_scope_index]) {
					continue;
				}
				flat_input_counter++;

				int second_block_sum = 0;
				for (int i = 0; i < 4; i++) {
					int value = rand()%4;
					zero_train_step(flat_input_counter,
									scope_starts[rand_scope_index],
									scope_ends[rand_scope_index],
									value,
									state_vals,
									scopes_on,
									zero_state_vals,
									zero_scopes_on,
									nodes,
									zero_nodes,
									sum_error);
					if (flat_input_counter == scope_ends[rand_scope_index]) {
						break;
					}
					flat_input_counter++;
					if (i < second_block_on_index) {
						second_block_sum += value;
					}
				}
				if (flat_input_counter == scope_ends[rand_scope_index]) {
					continue;
				}
			}
		}

		double max_update = 0.0;
		for (int n_index = 0; n_index < (int)zero_nodes.size(); n_index++) {
			zero_nodes[n_index]->calc_max_update(max_update, 0.0001);
		}
		double factor = 1.0;
		if (max_update > 0.001) {
			factor = 0.001/max_update;
		}
		for (int n_index = 0; n_index < (int)zero_nodes.size(); n_index++) {
			zero_nodes[n_index]->update_weights(factor, 0.0001);
		}
	}

	for (int n_index = 0; n_index < (int)zero_nodes.size(); n_index++) {
		ofstream output_file;
		output_file.open("saves/" + zero_nodes[n_index]->id + "_zt.txt");
		zero_nodes[n_index]->save(output_file);
		output_file.close();
	}

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	for (int n_index = 0; n_index < (int)zero_nodes.size(); n_index++) {
		delete zero_nodes[n_index];
	}

	cout << "Done" << endl;
}
