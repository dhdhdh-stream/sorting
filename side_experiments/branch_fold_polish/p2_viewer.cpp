/**
 * 0: blank
 * 1: 2nd val
 * 2: 1st val
 * 3: 1st xor
 * 4: 1st xor
 * 5: blank
 * 6: 2nd xor
 * 7: 2nd xor
 * 8: blank
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
double global_sum_error;

int id_counter;
mutex id_counter_mtx;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ifstream scope_save_file;
	scope_save_file.open("saves/scope_12.txt");
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
	for (int s_index = 0; s_index < inner_scope->sequence_length; s_index++) {
		cout << s_index << endl;
		cout << "inner_scope->is_inner_scope[s_index]: " << inner_scope->is_inner_scope[s_index] << endl;
		cout << "inner_scope->inner_input_networks[s_index].size(): " << inner_scope->inner_input_networks[s_index].size() << endl;
		cout << "inner_scope->compress_new_sizes[s_index]: " << inner_scope->compress_new_sizes[s_index] << endl;
		cout << "inner_scope->compress_original_sizes[s_index]: " << inner_scope->compress_original_sizes[s_index] << endl;
		cout << endl;
	}

	{
		vector<vector<double>> flat_vals;
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
		int val_2 = 1+rand()%2;
		flat_vals.push_back(vector<double>{(double)val_2});
		int val_1 = 1+rand()%2;
		flat_vals.push_back(vector<double>{(double)val_1});
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

		double target_val = (double)(val_1*((xor_1_1+xor_1_2)%2*2-1) + val_2*((xor_2_1+xor_2_2)%2*2-1));

		cout << "target_val: " << target_val << endl;

		vector<double> local_s_input_vals;
		vector<double> local_state_vals;

		double predicted_score = 0.0;
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
