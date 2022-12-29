/**
 * 0: blank
 * 1: 1st xor
 * 2: 1st xor
 * 3: blank
 * 4: 2nd xor
 * 5: 2nd xor
 * 6: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <random>

#include "definitions.h"
#include "fold.h"
#include "scope.h"

using namespace std;

default_random_engine generator;
double global_sum_error;

int id_counter;
mutex id_counter_mtx;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	vector<bool> is_existing(7, false);
	vector<Scope*> existing_actions(7, NULL);
	vector<int> obs_sizes(7, 1);

	double outer_misguess = 0.0;

	Fold* fold = new Fold(7,
						  is_existing,
						  existing_actions,
						  obs_sizes,
						  0,
						  &outer_misguess,
						  0,
						  0);

	while (true) {
		vector<vector<double>> flat_vals;
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
		int xor_1_1 = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)xor_1_1-1});
		int xor_1_2 = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)xor_1_2-1});
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
		int xor_2_1 = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)xor_2_1-1});
		int xor_2_2 = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)xor_2_2-1});
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});

		double target_val = (double)((xor_1_1+xor_1_2)%2*2-1 + (xor_2_1+xor_2_2)%2*2-1);

		outer_misguess = 0.999*outer_misguess + 0.001*(target_val - 0.0);

		double outer_score = 0.0;

		vector<double> outer_s_input_vals;
		vector<double> outer_state_vals;

		double predicted_score = 0.0;
		double scale_factor = 1.0;

		FoldHistory* fold_history = new FoldHistory(fold);
		fold->explore_on_path_activate(outer_score,
									   flat_vals,
									   outer_s_input_vals,
									   outer_state_vals,
									   predicted_score,
									   scale_factor,
									   fold_history);

		vector<double> outer_state_errors;

		double outer_scale_factor_error = 0.0;	// unused

		int explore_signal = fold->explore_on_path_backprop(
			outer_state_errors,
			predicted_score,
			target_val,
			scale_factor,
			outer_scale_factor_error,
			fold_history);
		delete fold_history;

		if (fold->state_iter%10000 == 0) {
			cout << "fold->state_iter: " << fold->state_iter << endl;
		}

		if (explore_signal != EXPLORE_SIGNAL_NONE) {
			break;
		}
	}

	cout << "Done" << endl;
}