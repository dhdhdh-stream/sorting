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
#include "fold_to_path.h"
#include "scope_path.h"

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
		if (fold.state_iter < 200000 && is_seed) {
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
		fold.explore_activate(flat_vals,
							  predicted_score);
		
		double final_misguess = abs(target_val - predicted_score);
		fold.explore_backprop(target_val,
							  final_misguess,
							  predicted_score);

		if (fold.state == STATE_EXPLORE_DONE) {
			break;
		}
	}

	for (int f_index = 0; f_index < fold.sequence_length; f_index++) {
		cout << f_index << ": " << fold.curr_step_impacts[f_index] << endl;
	}

	fold.explore_to_add();

	while (true) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

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

		double predicted_score = 0.0;
		fold.add_activate(flat_vals,
						  predicted_score);

		double final_misguess = abs(target_val - predicted_score);
		fold.add_backprop(target_val,
						  final_misguess,
						  predicted_score);

		if (fold.state == STATE_ADD_DONE) {
			break;
		}
	}

	for (int f_index = 0; f_index < fold.sequence_length; f_index++) {
		cout << f_index << ": " << fold.curr_step_impacts[f_index] << endl;
	}

	fold.add_to_clean();

	while (true) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

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

		double predicted_score = 0.0;
		fold.clean_activate(flat_vals,
							predicted_score);

		double final_misguess = abs(target_val - predicted_score);
		fold.clean_backprop(target_val,
							final_misguess,
							predicted_score);

		if (fold.state == STATE_DONE) {
			break;
		}
	}

	ScopePath* scope_path = fold_to_path(&fold);
	for (int i = 0; i < 10; i++) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

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

		double predicted_score = 0.0;

		vector<double> input_vals;	// empty
		scope_path->activate(input_vals,
							 flat_vals,
							 predicted_score);

		cout << "target_val: " << target_val << endl;
		cout << "predicted_score: " << predicted_score << endl;
		cout << endl;
	}

	delete scope_path;

	cout << "Done" << endl;
}
