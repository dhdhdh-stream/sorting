/**
 * 0 - 2: blank
 * 1 - 3: 1 which is index
 * 2 - 3: 2 which are 1st val
 * 3 - 2: 1 which is 1st val
 * 4 - 3: blank
 * 5 - 4: 1 which is 1st val
 * 6 - 1: blank
 * 7 - 3: 1 which is 2nd val
 * 8 - 2: 2 which is 2nd val
 * 9 - 3: blank
 * 10 - 2: 1 which is 2nd val
 * 11 - 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "scope.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<Node*> nodes;
	for (int i = 0; i < 12; i++) {
		ifstream input_file;
		input_file.open("saves/n_" + to_string(i) + "_11.txt");
		nodes.push_back(new Node(input_file));
		input_file.close();
	}

	Scope* scope = construct_scope(nodes);

	vector<Scope*> scope_dictionary;
	scope->add_to_dictionary(scope_dictionary);
	for (int sc_index = 0; sc_index < (int)scope_dictionary.size(); sc_index++) {
		cout << sc_index << endl;
		cout << "scope_dictionary[sc_index]->actions.size(): " << scope_dictionary[sc_index]->actions.size() << endl;
		cout << "scope_dictionary[sc_index]->num_inputs: " << scope_dictionary[sc_index]->num_inputs << endl;
		cout << "scope_dictionary[sc_index]->num_outputs: " << scope_dictionary[sc_index]->num_outputs << endl;
		cout << endl;
	}

	/**
	 * 2: blank
	 * 2: 1 which is index
	 * 4: scope (3+1)
	 * 2: blank
	 */

	vector<int> new_flat_sizes;
	new_flat_sizes.push_back(2);
	new_flat_sizes.push_back(2);
	new_flat_sizes.push_back(4);
	new_flat_sizes.push_back(2);
	FoldNetwork* new_fold_network = new FoldNetwork(new_flat_sizes);

	Network* flat_input_network = new Network(4, 50, 2);
	Scope* inner_scope = scope_dictionary[1];

	{
		double sum_error = 0.0;
		for (int iter_index = 1; iter_index < 500000; iter_index++) {
			if (iter_index%10000 == 0) {
				cout << endl;
				cout << iter_index << endl;
				cout << "sum_error: " << sum_error << endl;
				sum_error = 0.0;
			}

			vector<vector<double>> flat_vals;
			flat_vals.reserve(4);

			vector<double> flat_input_inputs;

			flat_vals.push_back(vector<double>(2));
			flat_vals[0][0] = rand()%2*2-1;
			flat_input_inputs.push_back(flat_vals[0][0]);
			flat_vals[0][1] = rand()%2*2-1;
			flat_input_inputs.push_back(flat_vals[0][1]);

			flat_vals.push_back(vector<double>(2));
			int index = rand()%4;
			flat_vals[1][0] = index;
			flat_input_inputs.push_back(flat_vals[1][0]);
			flat_vals[1][1] = rand()%2*2-1;
			flat_input_inputs.push_back(flat_vals[1][1]);
			int index_sum = 0;

			flat_input_network->activate(flat_input_inputs);
			vector<double> inner_scope_inputs(2);
			inner_scope_inputs[0] = flat_input_network->output->acti_vals[0];
			inner_scope_inputs[1] = flat_input_network->output->acti_vals[1];

			int scope_sum = 0;

			vector<vector<double>> inner_flat_vals;
			inner_flat_vals.reserve(4);

			inner_flat_vals.push_back(vector<double>(3));
			inner_flat_vals[0][0] = rand()%2*2-1;
			if (inner_flat_vals[0][0] == 1.0) {
				scope_sum++;
				if (index == 0) {
					index_sum++;
				}
			}
			inner_flat_vals[0][1] = rand()%2*2-1;
			if (inner_flat_vals[0][1] == 1.0) {
				scope_sum++;
				if (index == 1) {
					index_sum++;
				}
			}
			inner_flat_vals[0][2] = rand()%2*2-1;

			inner_flat_vals.push_back(vector<double>(2));
			inner_flat_vals[1][0] = rand()%2*2-1;
			if (inner_flat_vals[1][0] == 1.0) {
				scope_sum++;
				if (index == 2) {
					index_sum++;
				}
			}
			inner_flat_vals[1][1] = rand()%2*2-1;

			inner_flat_vals.push_back(vector<double>(3));
			inner_flat_vals[2][0] = rand()%2*2-1;
			inner_flat_vals[2][1] = rand()%2*2-1;
			inner_flat_vals[2][2] = rand()%2*2-1;

			inner_flat_vals.push_back(vector<double>(4));
			inner_flat_vals[3][0] = rand()%2*2-1;
			if (inner_flat_vals[3][0] == 1.0) {
				scope_sum++;
				if (index == 3) {
					index_sum++;
				}
			}
			inner_flat_vals[3][1] = rand()%2*2-1;
			inner_flat_vals[3][2] = rand()%2*2-1;
			inner_flat_vals[3][3] = rand()%2*2-1;

			double inner_predicted_score = 0.5;
			inner_scope->activate(inner_flat_vals,
								  inner_scope_inputs,
								  inner_predicted_score);

			flat_vals.push_back(vector<double>(4));
			flat_vals[2][0] = inner_scope->outputs[0];
			flat_vals[2][1] = inner_scope->outputs[1];
			flat_vals[2][2] = inner_scope->outputs[2];
			flat_vals[2][3] = inner_predicted_score;

			flat_vals.push_back(vector<double>(2));
			flat_vals[3][0] = rand()%2*2-1;
			flat_vals[3][1] = rand()%2*2-1;

			new_fold_network->activate(flat_vals);

			double final_val = scope_sum%2 + index_sum;

			vector<double> errors;
			errors.push_back(final_val - new_fold_network->output->acti_vals[0]);
			sum_error += abs(errors[0]);

			if (iter_index < 400000) {
				new_fold_network->backprop(errors, 0.01);
			} else {
				new_fold_network->backprop(errors, 0.001);
			}

			vector<double> inner_scope_errors(3);
			inner_scope_errors[0] = new_fold_network->flat_inputs[2]->errors[0];
			new_fold_network->flat_inputs[2]->errors[0] = 0.0;
			inner_scope_errors[1] = new_fold_network->flat_inputs[2]->errors[1];
			new_fold_network->flat_inputs[2]->errors[1] = 0.0;
			inner_scope_errors[2] = new_fold_network->flat_inputs[2]->errors[2];
			new_fold_network->flat_inputs[2]->errors[2] = 0.0;
			vector<double> inner_scope_output_errors;
			inner_scope->backprop(inner_scope_errors,
								  inner_scope_output_errors,
								  inner_predicted_score,
								  final_val);

			if (iter_index < 400000) {
				flat_input_network->backprop(inner_scope_output_errors, 0.01);
			} else {
				flat_input_network->backprop(inner_scope_output_errors, 0.001);
			}
		}
	}

	ofstream output_file;
	output_file.open("saves/new_fold_network.txt");
	new_fold_network->save(output_file);
	output_file.close();

	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		delete nodes[n_index];
	}
	delete scope;
	for (int sc_index = 0; sc_index < (int)scope_dictionary.size(); sc_index++) {
		delete scope_dictionary[sc_index];
	}
	delete flat_input_network;
	delete new_fold_network;

	cout << "Done" << endl;
}
