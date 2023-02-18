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

	while (true) {
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

		double predicted_score = 0.0;
		fold.flat_activate(flat_vals,
						   predicted_score);
		
		double final_misguess = target_val - predicted_score;
		fold.flat_backprop(target_val,
						   final_misguess,
						   predicted_score);

		if (fold.state == STATE_FLAT_DONE) {
			break;
		}
	}

	fold.flat_to_fold();

	while (true) {
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

		double predicted_score = 0.0;
		fold.fold_activate(flat_vals,
						   predicted_score);
		
		double final_misguess = target_val - predicted_score;
		fold.fold_backprop(target_val,
						   final_misguess,
						   predicted_score);

		if (fold.state == STATE_FOLD_DONE) {
			break;
		}
	}

	cout << "Done" << endl;
}
