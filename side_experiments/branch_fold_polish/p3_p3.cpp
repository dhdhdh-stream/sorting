/**
 * Note: difficult to simply reverse because branch chooses high score? but mostly works
 * blank
 * -
 * -2 * scope
 * -
 * blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <mutex>
#include <random>

#include "definitions.h"
#include "fold.h"
#include "fold_to_path.h"
#include "scope.h"

using namespace std;

default_random_engine generator;
bool global_debug_flag = false;
double global_sum_error = 0.0;

int id_counter = 40;
mutex id_counter_mtx;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ifstream scope_save_file;
	scope_save_file.open("saves/scope_8.txt");
	Scope* scope = new Scope(scope_save_file);
	scope_save_file.close();

	for (int s_index = 0; s_index < scope->sequence_length; s_index++) {
		cout << s_index << endl;
		cout << "scope->is_inner_scope[s_index]: " << scope->is_inner_scope[s_index] << endl;
		cout << "scope->inner_input_networks[s_index].size(): " << scope->inner_input_networks[s_index].size() << endl;
		cout << "scope->compress_new_sizes[s_index]: " << scope->compress_new_sizes[s_index] << endl;
		cout << "scope->compress_original_sizes[s_index]: " << scope->compress_original_sizes[s_index] << endl;
		cout << endl;
	}

	Scope* inner_scope = scope->scopes[1];

	cout << "inner_scope->num_inputs: " << inner_scope->num_inputs << endl;
	cout << "inner_scope->num_outputs: " << inner_scope->num_outputs << endl;

	vector<bool> is_existing{
		false,
		true,
		false,
	};
	vector<Scope*> existing_actions{
		NULL,
		inner_scope,
		NULL,
	};
	vector<int> obs_sizes(3, 1);

	double outer_average_score = 0.0;
	double outer_average_misguess = 0.0;

	Fold* fold = new Fold(3,
						  is_existing,
						  existing_actions,
						  obs_sizes,
						  0,
						  &outer_average_score,
						  &outer_average_misguess,
						  0,
						  0);

	while (true) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});

		{
			int switch_val = rand()%2;
			flat_vals.push_back(vector<double>{2*(double)switch_val-1});
			if (switch_val == 0) {
				if (fold->state_iter%10000 == 0) {
					cout << "case #2" << endl;
				}

				int xor_1 = rand()%2;
				flat_vals.push_back(vector<double>{2*(double)xor_1-1});
				int xor_2 = rand()%2;
				flat_vals.push_back(vector<double>{2*(double)xor_2-1});

                // target_val += -2.0*((xor_1+xor_2)%2);
                target_val += 2.0*((xor_1+xor_2)%2);
			} else {
				if (fold->state_iter%10000 == 0) {
					cout << "case #1" << endl;
				}

				flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
				int xor_1 = rand()%2;
				flat_vals.push_back(vector<double>{2*(double)xor_1-1});
				int xor_2 = rand()%2;
				flat_vals.push_back(vector<double>{2*(double)xor_2-1});

                // target_val += -2.0*((xor_1+xor_2)%2);
                target_val += 2.0*((xor_1+xor_2)%2);
			}
		}

		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});

		// padding
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});

		if (fold->state_iter%10000 == 0) {
			cout << "target_val: " << target_val << endl;
			global_debug_flag = true;
		}

		outer_average_score = 0.999*outer_average_score + 0.001*target_val;

		double curr_misguess = (target_val - outer_average_score)*(target_val - outer_average_score);
		outer_average_misguess = 0.999*outer_average_misguess + 0.001*curr_misguess;

		double outer_score = -1.0;

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

		if (fold->state_iter%10000 == 0) {
			cout << "predicted_score: " << predicted_score << endl;
			global_debug_flag = false;
		}

		vector<double> outer_state_errors;

		int explore_signal = fold->explore_on_path_backprop(
			outer_state_errors,
			predicted_score,
			target_val,
			scale_factor,
			fold_history);
		delete fold_history;

		if (explore_signal != EXPLORE_SIGNAL_NONE) {
			break;
		}
	}

	delete fold;
	delete scope;

	cout << "Done" << endl;
}
