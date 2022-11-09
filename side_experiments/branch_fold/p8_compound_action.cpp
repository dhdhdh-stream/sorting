/**
 * 0 - 2: blank
 * 1 - 2: 1 which is index val
 * 2 - 2: 1 which is index val
 * 3 - 2: blank
 *   0 - 2: 1 which is 1st val
 *   1 - 2: 1 which is 1st val
 *   2 - 3: 1 which is 1st val
 * 5 - 2: blank
 *   0 - 2: 1 which is 2nd val
 *   1 - 2: 1 which is 2nd val
 *   2 - 3: 1 which is 2nd val
 * 7 - 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold.h"
#include "fold_network.h"
#include "network.h"
#include "node.h"
#include "scope.h"
#include "test_node.h"

using namespace std;

default_random_engine generator;
double global_sum_error;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// vector<Node*> nodes;
	// for (int i = 0; i < 8; i++) {
	// 	ifstream input_file;
	// 	input_file.open("saves/fold_n_" + to_string(i) + ".txt");
	// 	nodes.push_back(new Node(input_file));
	// 	input_file.close();
	// }

	// Scope* scope = construct_scope(nodes);

	// vector<Scope*> scope_dictionary;
	// scope->add_to_dictionary(scope_dictionary);

	// for (int s_index = 0; s_index < (int)scope_dictionary.size(); s_index++) {
	// 	cout << s_index << endl;
	// 	cout << "actions: " << scope_dictionary[s_index]->actions.size() << endl;
	// 	cout << "num_inputs: "  << scope_dictionary[s_index]->num_inputs << endl;
	// 	cout << "num_outputs: " << scope_dictionary[s_index]->num_outputs << endl;
	// 	cout << endl;
	// }

	// scope_dictionary[1]->id = "original_scope";
	// ofstream original_scope_save_file;
	// original_scope_save_file.open("saves/" + scope_dictionary[1]->id + ".txt");
	// scope_dictionary[1]->save(original_scope_save_file);
	// original_scope_save_file.close();

	// for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
	//  delete nodes[n_index];
	// }
	// delete scope;

	ifstream original_scope_save_file;
	original_scope_save_file.open("saves/original_scope.txt");
	Scope* original_scope = new Scope(original_scope_save_file);
	original_scope_save_file.close();

	vector<int> flat_sizes;
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(2);
	flat_sizes.push_back(3);
	flat_sizes.push_back(2);
	flat_sizes.push_back(3);
	flat_sizes.push_back(2);
	FoldNetwork* fold_network = new FoldNetwork(flat_sizes, 1);

	vector<int> scope_input_1_flat_sizes;
	scope_input_1_flat_sizes.push_back(2);
	scope_input_1_flat_sizes.push_back(2);
	scope_input_1_flat_sizes.push_back(2);
	scope_input_1_flat_sizes.push_back(2);
	FoldNetwork* scope_input_network_1 = new FoldNetwork(scope_input_1_flat_sizes, 2);
	Scope* scope_1 = new Scope(original_scope);

	vector<int> scope_input_2_flat_sizes;
	scope_input_2_flat_sizes.push_back(2);
	scope_input_2_flat_sizes.push_back(2);
	scope_input_2_flat_sizes.push_back(2);
	scope_input_2_flat_sizes.push_back(2);
	scope_input_2_flat_sizes.push_back(3);
	scope_input_2_flat_sizes.push_back(2);
	FoldNetwork* scope_input_network_2 = new FoldNetwork(scope_input_2_flat_sizes, 2);
	Scope* scope_2 = new Scope(original_scope);

	double sum_error = 0.0;
	for (int iter_index = 1; iter_index < 500000; iter_index++) {
		if (iter_index%10000 == 0) {
			cout << endl;
			cout << iter_index << endl;
			cout << "sum_error: " << sum_error << endl;
			sum_error = 0.0;
		}

		double final_val = 0;

		vector<vector<double>> flat_vals;
		flat_vals.reserve(8);

		vector<vector<double>> scope_input_1_flat_vals;
		scope_input_1_flat_vals.reserve(4);

		vector<vector<double>> scope_input_2_flat_vals;
		scope_input_2_flat_vals.reserve(6);

		flat_vals.push_back(vector<double>(2));
		flat_vals[0][0] = rand()%2*2-1;
		flat_vals[0][1] = rand()%2*2-1;
		scope_input_1_flat_vals.push_back(vector<double>(2));
		scope_input_1_flat_vals[0][0] = flat_vals[0][0];
		scope_input_1_flat_vals[0][1] = flat_vals[0][1];
		scope_input_2_flat_vals.push_back(vector<double>(2));
		scope_input_2_flat_vals[0][0] = flat_vals[0][0];
		scope_input_2_flat_vals[0][1] = flat_vals[0][1];

		int index = 0;

		flat_vals.push_back(vector<double>(2));
		flat_vals[1][0] = rand()%2*2-1;
		if (flat_vals[1][0] == 1.0) {
			index++;
		}
		flat_vals[1][1] = rand()%2*2-1;
		scope_input_1_flat_vals.push_back(vector<double>(2));
		scope_input_1_flat_vals[1][0] = flat_vals[1][0];
		scope_input_1_flat_vals[1][1] = flat_vals[1][1];
		scope_input_2_flat_vals.push_back(vector<double>(2));
		scope_input_2_flat_vals[1][0] = flat_vals[1][0];
		scope_input_2_flat_vals[1][1] = flat_vals[1][1];

		flat_vals.push_back(vector<double>(2));
		flat_vals[2][0] = rand()%2*2-1;
		if (flat_vals[2][0] == 1.0) {
			index++;
		}
		flat_vals[2][1] = rand()%2*2-1;
		scope_input_1_flat_vals.push_back(vector<double>(2));
		scope_input_1_flat_vals[2][0] = flat_vals[2][0];
		scope_input_1_flat_vals[2][1] = flat_vals[2][1];
		scope_input_2_flat_vals.push_back(vector<double>(2));
		scope_input_2_flat_vals[2][0] = flat_vals[2][0];
		scope_input_2_flat_vals[2][1] = flat_vals[2][1];

		flat_vals.push_back(vector<double>(2));
		flat_vals[3][0] = rand()%2*2-1;
		flat_vals[3][1] = rand()%2*2-1;
		scope_input_1_flat_vals.push_back(vector<double>(2));
		scope_input_1_flat_vals[3][0] = flat_vals[3][0];
		scope_input_1_flat_vals[3][1] = flat_vals[3][1];
		scope_input_2_flat_vals.push_back(vector<double>(2));
		scope_input_2_flat_vals[3][0] = flat_vals[3][0];
		scope_input_2_flat_vals[3][1] = flat_vals[3][1];

		scope_input_network_1->activate(scope_input_1_flat_vals);
		vector<double> scope_1_input(2);
		scope_1_input[0] = scope_input_network_1->output->acti_vals[0];
		scope_1_input[1] = scope_input_network_1->output->acti_vals[1];

		vector<vector<double>> inner_scope_1_flat_vals;
		inner_scope_1_flat_vals.reserve(3);

		inner_scope_1_flat_vals.push_back(vector<double>(2));
		inner_scope_1_flat_vals[0][0] = rand()%2*2-1;
		if (index == 0) {
			final_val += inner_scope_1_flat_vals[0][0];
		}
		final_val += inner_scope_1_flat_vals[0][0];
		inner_scope_1_flat_vals[0][1] = rand()%2*2-1;

		inner_scope_1_flat_vals.push_back(vector<double>(2));
		inner_scope_1_flat_vals[1][0] = rand()%2*2-1;
		if (index == 1) {
			final_val += inner_scope_1_flat_vals[1][0];
		}
		final_val += inner_scope_1_flat_vals[1][0];
		inner_scope_1_flat_vals[1][1] = rand()%2*2-1;

		inner_scope_1_flat_vals.push_back(vector<double>(3));
		inner_scope_1_flat_vals[2][0] = rand()%2*2-1;
		if (index == 2) {
			final_val += inner_scope_1_flat_vals[2][0];
		}
		final_val += inner_scope_1_flat_vals[2][0];
		inner_scope_1_flat_vals[2][1] = rand()%2*2-1;
		inner_scope_1_flat_vals[2][2] = rand()%2*2-1;

		double scope_1_predicted_score = 0.0;
		scope_1->activate(inner_scope_1_flat_vals,
						  scope_1_input,
						  scope_1_predicted_score);

		flat_vals.push_back(vector<double>(3));
		flat_vals[4][0] = scope_1->outputs[0];
		flat_vals[4][1] = scope_1->outputs[1];
		flat_vals[4][2] = scope_1_predicted_score;
		scope_input_2_flat_vals.push_back(vector<double>(3));
		scope_input_2_flat_vals[4][0] = flat_vals[4][0];
		scope_input_2_flat_vals[4][1] = flat_vals[4][1];
		scope_input_2_flat_vals[4][2] = flat_vals[4][2];

		flat_vals.push_back(vector<double>(2));
		flat_vals[5][0] = rand()%2*2-1;
		flat_vals[5][1] = rand()%2*2-1;
		scope_input_2_flat_vals.push_back(vector<double>(2));
		scope_input_2_flat_vals[5][0] = flat_vals[5][0];
		scope_input_2_flat_vals[5][1] = flat_vals[5][1];

		scope_input_network_2->activate(scope_input_2_flat_vals);
		vector<double> scope_2_input(2);
		scope_2_input[0] = scope_input_network_2->output->acti_vals[0];
		scope_2_input[1] = scope_input_network_2->output->acti_vals[1];

		vector<vector<double>> inner_scope_2_flat_vals;
		inner_scope_2_flat_vals.reserve(3);

		inner_scope_2_flat_vals.push_back(vector<double>(2));
		inner_scope_2_flat_vals[0][0] = rand()%2*2-1;
		if (index == 0) {
			final_val += inner_scope_2_flat_vals[0][0];
		}
		final_val += inner_scope_2_flat_vals[0][0];
		inner_scope_2_flat_vals[0][1] = rand()%2*2-1;

		inner_scope_2_flat_vals.push_back(vector<double>(2));
		inner_scope_2_flat_vals[1][0] = rand()%2*2-1;
		if (index == 1) {
			final_val += inner_scope_2_flat_vals[1][0];
		}
		final_val += inner_scope_2_flat_vals[1][0];
		inner_scope_2_flat_vals[1][1] = rand()%2*2-1;

		inner_scope_2_flat_vals.push_back(vector<double>(3));
		inner_scope_2_flat_vals[2][0] = rand()%2*2-1;
		if (index == 2) {
			final_val += inner_scope_2_flat_vals[2][0];
		}
		final_val += inner_scope_2_flat_vals[2][0];
		inner_scope_2_flat_vals[2][1] = rand()%2*2-1;
		inner_scope_2_flat_vals[2][2] = rand()%2*2-1;

		double scope_2_predicted_score = 0.0;
		scope_2->activate(inner_scope_2_flat_vals,
						  scope_2_input,
						  scope_2_predicted_score);

		flat_vals.push_back(vector<double>(3));
		flat_vals[6][0] = scope_2->outputs[0];
		flat_vals[6][1] = scope_2->outputs[1];
		flat_vals[6][2] = scope_2_predicted_score;

		flat_vals.push_back(vector<double>(2));
		flat_vals[7][0] = rand()%2*2-1;
		flat_vals[7][1] = rand()%2*2-1;

		fold_network->activate(flat_vals);

		vector<double> errors;
		errors.push_back(final_val - fold_network->output->acti_vals[0]);
		sum_error += abs(errors[0]);

		if (iter_index < 300000) {
			fold_network->backprop(errors, 0.05);
		} else if (iter_index < 400000) {
			fold_network->backprop(errors, 0.01);
		} else {
			fold_network->backprop(errors, 0.001);
		}

		vector<double> scope_2_input_errors(2);
		scope_2_input_errors[0] = fold_network->flat_inputs[6]->errors[0];
		fold_network->flat_inputs[6]->errors[0] = 0.0;
		scope_2_input_errors[1] = fold_network->flat_inputs[6]->errors[1];
		fold_network->flat_inputs[6]->errors[1] = 0.0;
		double scope_2_predicted_score_error = fold_network->flat_inputs[6]->errors[2];
		fold_network->flat_inputs[6]->errors[2] = 0.0;
		vector<double> scope_2_output_errors;
		scope_2->backprop_errors_with_no_weight_change(
		// scope_2->backprop(
			scope_2_input_errors,
			scope_2_output_errors,
			scope_2_predicted_score_error);

		if (iter_index < 300000) {
			scope_input_network_2->backprop(scope_2_output_errors, 0.05);
		} else if (iter_index < 400000) {
			scope_input_network_2->backprop(scope_2_output_errors, 0.01);
		} else {
			scope_input_network_2->backprop(scope_2_output_errors, 0.001);
		}

		vector<double> scope_1_input_errors(2, 0.0);
		scope_1_input_errors[0] += fold_network->flat_inputs[4]->errors[0];
		fold_network->flat_inputs[4]->errors[0] = 0.0;
		scope_1_input_errors[0] += scope_input_network_2->flat_inputs[4]->errors[0];
		scope_input_network_2->flat_inputs[4]->errors[0] = 0.0;
		scope_1_input_errors[1] += fold_network->flat_inputs[4]->errors[1];
		fold_network->flat_inputs[4]->errors[1] = 0.0;
		scope_1_input_errors[1] += scope_input_network_2->flat_inputs[4]->errors[1];
		scope_input_network_2->flat_inputs[4]->errors[1] = 0.0;
		double scope_1_predicted_score_error = 0.0;
		scope_1_predicted_score_error += fold_network->flat_inputs[4]->errors[2];
		fold_network->flat_inputs[4]->errors[2] = 0.0;
		scope_1_predicted_score_error += scope_input_network_2->flat_inputs[4]->errors[2];
		scope_input_network_2->flat_inputs[4]->errors[2] = 0.0;
		vector<double> scope_1_output_errors;
		scope_1->backprop_errors_with_no_weight_change(
		// scope_1->backprop(
			scope_1_input_errors,
			scope_1_output_errors,
			scope_1_predicted_score_error);

		if (iter_index < 300000) {
			scope_input_network_1->backprop(scope_1_output_errors, 0.05);
		} else if (iter_index < 400000) {
			scope_input_network_1->backprop(scope_1_output_errors, 0.01);
		} else {
			scope_input_network_1->backprop(scope_1_output_errors, 0.001);
		}
	}

	double average_score = 0.0;

	vector<Scope*> compound_actions(8);
	compound_actions[0] = NULL;
	compound_actions[1] = NULL;
	compound_actions[2] = NULL;
	compound_actions[3] = NULL;
	compound_actions[4] = scope_1;
	compound_actions[5] = NULL;
	compound_actions[6] = scope_2;
	compound_actions[7] = NULL;
	vector<int> obs_sizes(8);
	obs_sizes[0] = 2;
	obs_sizes[1] = 2;
	obs_sizes[2] = 2;
	obs_sizes[3] = 2;
	obs_sizes[4] = -1;
	obs_sizes[5] = 2;
	obs_sizes[6] = -1;
	obs_sizes[7] = 2;
	vector<FoldNetwork*> original_input_folds(8);
	original_input_folds[0] = NULL;
	original_input_folds[1] = NULL;
	original_input_folds[2] = NULL;
	original_input_folds[3] = NULL;
	original_input_folds[4] = scope_input_network_1;
	original_input_folds[5] = NULL;
	original_input_folds[6] = scope_input_network_2;
	original_input_folds[7] = NULL;
	Fold fold("fold_2",
			  8,
			  compound_actions,
			  obs_sizes,
			  average_score,
			  0.02,
			  fold_network,
			  original_input_folds);

	while (true) {
		double final_val = 0;

		vector<vector<double>> flat_vals;
		flat_vals.reserve(8);
		vector<vector<vector<double>>> inner_flat_vals(8);

		flat_vals.push_back(vector<double>(2));
		flat_vals[0][0] = rand()%2*2-1;
		flat_vals[0][1] = rand()%2*2-1;

		int index = 0;

		flat_vals.push_back(vector<double>(2));
		flat_vals[1][0] = rand()%2*2-1;
		if (flat_vals[1][0] == 1.0) {
			index++;
		}
		flat_vals[1][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[2][0] = rand()%2*2-1;
		if (flat_vals[2][0] == 1.0) {
			index++;
		}
		flat_vals[2][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[3][0] = rand()%2*2-1;
		flat_vals[3][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>());	// push empty
		inner_flat_vals[4].reserve(3);

		inner_flat_vals[4].push_back(vector<double>(2));
		inner_flat_vals[4][0][0] = rand()%2*2-1;
		if (index == 0) {
			final_val += inner_flat_vals[4][0][0];
		}
		final_val += inner_flat_vals[4][0][0];
		inner_flat_vals[4][0][1] = rand()%2*2-1;

		inner_flat_vals[4].push_back(vector<double>(2));
		inner_flat_vals[4][1][0] = rand()%2*2-1;
		if (index == 1) {
			final_val += inner_flat_vals[4][1][0];
		}
		final_val += inner_flat_vals[4][1][0];
		inner_flat_vals[4][1][1] = rand()%2*2-1;

		inner_flat_vals[4].push_back(vector<double>(3));
		inner_flat_vals[4][2][0] = rand()%2*2-1;
		if (index == 2) {
			final_val += inner_flat_vals[4][2][0];
		}
		final_val += inner_flat_vals[4][2][0];
		inner_flat_vals[4][2][1] = rand()%2*2-1;
		inner_flat_vals[4][2][2] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[5][0] = rand()%2*2-1;
		flat_vals[5][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>());	// push empty
		inner_flat_vals[6].reserve(3);

		inner_flat_vals[6].push_back(vector<double>(2));
		inner_flat_vals[6][0][0] = rand()%2*2-1;
		if (index == 0) {
			final_val += inner_flat_vals[6][0][0];
		}
		final_val += inner_flat_vals[6][0][0];
		inner_flat_vals[6][0][1] = rand()%2*2-1;

		inner_flat_vals[6].push_back(vector<double>(2));
		inner_flat_vals[6][1][0] = rand()%2*2-1;
		if (index == 1) {
			final_val += inner_flat_vals[6][1][0];
		}
		final_val += inner_flat_vals[6][1][0];
		inner_flat_vals[6][1][1] = rand()%2*2-1;

		inner_flat_vals[6].push_back(vector<double>(3));
		inner_flat_vals[6][2][0] = rand()%2*2-1;
		if (index == 2) {
			final_val += inner_flat_vals[6][2][0];
		}
		final_val += inner_flat_vals[6][2][0];
		inner_flat_vals[6][2][1] = rand()%2*2-1;
		inner_flat_vals[6][2][2] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[7][0] = rand()%2*2-1;
		flat_vals[7][1] = rand()%2*2-1;

		fold.process(flat_vals,
					 inner_flat_vals,
					 final_val);

		if (fold.state == STATE_DONE) {
			break;
		}
	}

	delete original_scope;
	for (int n_index = 0; n_index < (int)fold.nodes.size(); n_index++) {
		delete fold.nodes[n_index];
	}
	delete fold.curr_fold;
	for (int f_index = 0; f_index < (int)fold.curr_scope_input_folds.size(); f_index++) {
		if (fold.curr_scope_input_folds[f_index] != NULL) {
			delete fold.curr_scope_input_folds[f_index];
		}
	}

	cout << "Done" << endl;
}
