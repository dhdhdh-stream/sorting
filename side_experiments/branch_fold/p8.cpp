/**
 * 0 - 2: blank
 * 1 - 2: 1 which is index val
 * 2 - 2: 1 which is val
 * 3 - 2: 1 which is val
 * 4 - 3: 1 which is val
 * 5 - 2: blank
 * 6 - 2: 1 which is clear val
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

	vector<int> initial_flat_sizes;
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(3);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	initial_flat_sizes.push_back(2);
	// FoldNetwork* initial_fold_network = new FoldNetwork(initial_flat_sizes, 1);

	// double sum_error = 0.0;
	// for (int iter_index = 1; iter_index < 500000; iter_index++) {
	// 	if (iter_index%10000 == 0) {
	// 		cout << endl;
	// 		cout << iter_index << endl;
	// 		cout << "sum_error: " << sum_error << endl;
	// 		sum_error = 0.0;
	// 	}

	// 	double final_val = 0;

	// 	vector<vector<double>> flat_vals;
	// 	flat_vals.reserve(8);

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[0][0] = rand()%2*2-1;
	// 	flat_vals[0][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	int index = rand()%3;
	// 	flat_vals[1][0] = index;
	// 	flat_vals[1][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[2][0] = rand()%2*2-1;
	// 	if (index == 0) {
	// 		final_val += flat_vals[2][0];
	// 	}
	// 	final_val += flat_vals[2][0];
	// 	flat_vals[2][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[3][0] = rand()%2*2-1;
	// 	if (index == 1) {
	// 		final_val += flat_vals[3][0];
	// 	}
	// 	final_val += flat_vals[3][0];
	// 	flat_vals[3][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(3));
	// 	flat_vals[4][0] = rand()%2*2-1;
	// 	if (index == 2) {
	// 		final_val += flat_vals[4][0];
	// 	}
	// 	final_val += flat_vals[4][0];
	// 	flat_vals[4][1] = rand()%2*2-1;
	// 	flat_vals[4][2] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[5][0] = rand()%2*2-1;
	// 	flat_vals[5][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[6][0] = rand()%2*2-1;
	// 	if (flat_vals[6][0] == 1.0) {
	// 		final_val = 0.0;
	// 	}
	// 	flat_vals[6][1] = rand()%2*2-1;

	// 	flat_vals.push_back(vector<double>(2));
	// 	flat_vals[7][0] = rand()%2*2-1;
	// 	flat_vals[7][1] = rand()%2*2-1;

	// 	initial_fold_network->activate(flat_vals);

	// 	vector<double> errors;
	// 	errors.push_back(final_val - initial_fold_network->output->acti_vals[0]);
	// 	sum_error += abs(errors[0]);

	// 	if (iter_index < 300000) {
	// 		initial_fold_network->backprop(errors, 0.05);
	// 	} else if (iter_index < 400000) {
	// 		initial_fold_network->backprop(errors, 0.01);
	// 	} else {
	// 		initial_fold_network->backprop(errors, 0.001);
	// 	}
	// }

	// ofstream output_file;
	// output_file.open("saves/initial_fold_network.txt");
	// initial_fold_network->save(output_file);
	// output_file.close();

	ifstream input_file;
	input_file.open("saves/initial_fold_network.txt");
	// input_file.open("saves/f_2.txt");
	FoldNetwork* initial_fold_network = new FoldNetwork(input_file);
	input_file.close();

	double average_score = 0.0;

	vector<Scope*> compound_actions(8, NULL);
	vector<int> obs_sizes(8);
	obs_sizes[0] = 2;
	obs_sizes[1] = 2;
	obs_sizes[2] = 2;
	obs_sizes[3] = 2;
	obs_sizes[4] = 3;
	obs_sizes[5] = 2;
	obs_sizes[6] = 2;
	obs_sizes[7] = 2;
	vector<FoldNetwork*> original_input_folds(8, NULL);
	Fold fold("fold",
			  8,
			  compound_actions,
			  obs_sizes,
			  average_score,
			  0.01,
			  initial_fold_network,
			  original_input_folds);

	while (true) {
		double final_val = 0;

		vector<vector<double>> flat_vals;
		flat_vals.reserve(8);
		vector<vector<vector<double>>> inner_flat_vals(8);

		flat_vals.push_back(vector<double>(2));
		flat_vals[0][0] = rand()%2*2-1;
		flat_vals[0][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		int index = rand()%3;
		flat_vals[1][0] = index;
		flat_vals[1][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[2][0] = rand()%2*2-1;
		if (index == 0) {
			final_val += flat_vals[2][0];
		}
		final_val += flat_vals[2][0];
		flat_vals[2][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[3][0] = rand()%2*2-1;
		if (index == 1) {
			final_val += flat_vals[3][0];
		}
		final_val += flat_vals[3][0];
		flat_vals[3][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(3));
		flat_vals[4][0] = rand()%2*2-1;
		if (index == 2) {
			final_val += flat_vals[4][0];
		}
		final_val += flat_vals[4][0];
		flat_vals[4][1] = rand()%2*2-1;
		flat_vals[4][2] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[5][0] = rand()%2*2-1;
		flat_vals[5][1] = rand()%2*2-1;

		flat_vals.push_back(vector<double>(2));
		flat_vals[6][0] = rand()%2*2-1;
		if (flat_vals[6][0] == 1.0) {
			final_val = 0.0;
		}
		flat_vals[6][1] = rand()%2*2-1;

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

	for (int n_index = 0; n_index < (int)fold.nodes.size(); n_index++) {
		ofstream output_file;
		output_file.open("saves/" + fold.nodes[n_index]->id + ".txt");
		fold.nodes[n_index]->save(output_file);
		output_file.close();
	}

	// delete initial_fold_network;

	cout << "Done" << endl;
}
