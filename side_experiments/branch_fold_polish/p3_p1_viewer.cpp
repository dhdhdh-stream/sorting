/**
 * 0: blank
 * 1: switch
 * 2: blank
 * 3: xor
 * 4: xor
 * 5: blank
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

int id_counter;
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

	{
		vector<vector<double>> flat_vals;
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
		int switch_val = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)switch_val-1});
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
		int xor_1 = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)xor_1-1});
		int xor_2 = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)xor_2-1});
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});

		double target_val;
		if (switch_val == 0) {
			target_val = -1;
		} else {
			target_val = (double)((xor_1+xor_2)%2);
		}

		cout << "target_val: " << target_val << endl;

		vector<double> local_s_input_vals;
		vector<double> local_state_vals;

		double predicted_score = -0.25;
		double scale_factor = 1.0;

		ScopeHistory* scope_history = new ScopeHistory(scope);
		scope->update_activate(flat_vals,
							   local_s_input_vals,
							   local_state_vals,
							   predicted_score,
							   scale_factor,
							   scope_history);

		cout << "predicted_score: " << predicted_score << endl;

		double next_predicted_score = predicted_score;

		scope->update_backprop(predicted_score,
							next_predicted_score,
							target_val,
							scale_factor,
							scope_history);
		delete scope_history;
	}

	delete scope;

	cout << "Done" << endl;
}
