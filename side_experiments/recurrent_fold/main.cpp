/**
 * 0: blank
 * 1: 1st xor
 * 2: 1st xor
 * 3: blank
 * 4: 2nd xor
 * 5: blank
 * 6: 2nd xor
 * 7: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "fold.h"

using namespace std;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	cout << "Seed: " << seed << endl;

	Fold fold(8);

	for (int iter = 0; iter < 1000000; iter++) {
		bool is_seed;
		if (rand()%10 == 0) {
			is_seed = true;
		} else {
			is_seed = false;
		}

		vector<vector<double>> flat_vals;
		double target_val = 0.0;
		if (is_seed) {
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});

			target_val = 2.0;
		} else {
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
			int xor_val_1_1 = rand()%2;
			flat_vals.push_back(vector<double>{(double)(xor_val_1_1*2-1)});
			int xor_val_1_2 = rand()%2;
			flat_vals.push_back(vector<double>{(double)(xor_val_1_2*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
			int xor_val_2_1 = rand()%2;
			flat_vals.push_back(vector<double>{(double)(xor_val_2_1*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
			int xor_val_2_2 = rand()%2;
			flat_vals.push_back(vector<double>{(double)(xor_val_2_2*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

			if ((xor_val_1_1+xor_val_1_2)%2 == 0) {
				target_val += 1.0;
			} else {
				target_val -= 1.0;
			}
			if ((xor_val_2_1+xor_val_2_2)%2 == 0) {
				target_val += 1.0;
			} else {
				target_val -= 1.0;
			}
		}

		fold.activate(flat_vals);
		fold.backprop(target_val);

		if (iter%10000 == 0) {
			cout << iter << endl;
			cout << "sum_error: " << fold.sum_error << endl;
			fold.sum_error = 0.0;
			cout << "target_val: " << target_val << endl;
			cout << "output: " << fold.outer_network->output->acti_vals[0] << endl;
			cout << endl;
		}
	}

	fold.add_state();

	for (int iter = 0; iter < 1000000; iter++) {
		bool is_seed;
		if (rand()%10 == 0) {
			is_seed = true;
		} else {
			is_seed = false;
		}

		vector<vector<double>> flat_vals;
		double target_val = 0.0;
		if (is_seed) {
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});
			flat_vals.push_back(vector<double>{1.0});

			target_val = 2.0;
		} else {
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
			int xor_val_1_1 = rand()%2;
			flat_vals.push_back(vector<double>{(double)(xor_val_1_1*2-1)});
			int xor_val_1_2 = rand()%2;
			flat_vals.push_back(vector<double>{(double)(xor_val_1_2*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
			int xor_val_2_1 = rand()%2;
			flat_vals.push_back(vector<double>{(double)(xor_val_2_1*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
			int xor_val_2_2 = rand()%2;
			flat_vals.push_back(vector<double>{(double)(xor_val_2_2*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

			if ((xor_val_1_1+xor_val_1_2)%2 == 0) {
				target_val += 1.0;
			} else {
				target_val -= 1.0;
			}
			if ((xor_val_2_1+xor_val_2_2)%2 == 0) {
				target_val += 1.0;
			} else {
				target_val -= 1.0;
			}
		}

		fold.activate(flat_vals);
		fold.backprop(target_val);

		if (iter%10000 == 0) {
			cout << iter << endl;
			cout << "sum_error: " << fold.sum_error << endl;
			fold.sum_error = 0.0;
			cout << "target_val: " << target_val << endl;
			cout << "output: " << fold.outer_network->output->acti_vals[0] << endl;
			cout << endl;
		}
	}

	cout << "Done" << endl;
}
