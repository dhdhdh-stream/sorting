/**
 * 0: blank
 * -
 * 1: switch
 * 2: xor
 * 3: xor
 * -
 * 4: blank
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

int id_counter = 20;
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

	scope->explore_index_inclusive = 1;
	scope->explore_type = EXPLORE_TYPE_INNER_SCOPE;
	scope->explore_end_non_inclusive = -1;    // doesn't matter
	scope->explore_fold = NULL;

	Scope* inner_scope = scope->scopes[1];

	for (int s_index = 0; s_index < inner_scope->sequence_length; s_index++) {
		cout << s_index << endl;
		cout << "inner_scope->is_inner_scope[s_index]: " << inner_scope->is_inner_scope[s_index] << endl;
		cout << "inner_scope->inner_input_networks[s_index].size(): " << inner_scope->inner_input_networks[s_index].size() << endl;
		cout << "inner_scope->compress_new_sizes[s_index]: " << inner_scope->compress_new_sizes[s_index] << endl;
		cout << "inner_scope->compress_original_sizes[s_index]: " << inner_scope->compress_original_sizes[s_index] << endl;
		cout << endl;
	}
	cout << "inner_scope->num_inputs: " << inner_scope->num_inputs << endl;
	cout << "inner_scope->num_outputs: " << inner_scope->num_outputs << endl;

	vector<bool> is_existing(2, false);
	vector<Scope*> existing_actions(2, NULL);
	vector<int> obs_sizes(2, 1);

	Fold* fold = new Fold(2,
						  is_existing,
						  existing_actions,
						  obs_sizes,
						  2,
						  &inner_scope->average_scores[0],
						  &inner_scope->average_misguesses[0],
						  0,
						  1);

	inner_scope->explore_index_inclusive = 0;
	inner_scope->explore_type = EXPLORE_TYPE_NEW;
	inner_scope->explore_end_non_inclusive = 4;
	inner_scope->explore_fold = fold;

	while (true) {
		vector<vector<double>> flat_vals;
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
		int switch_val = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)switch_val-1});
		int xor_1 = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)xor_1-1});
		int xor_2 = rand()%2;
		flat_vals.push_back(vector<double>{2*(double)xor_2-1});
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});

		double target_val;
		if (switch_val == 0) {
			target_val = (double)((xor_1+xor_2)%2);
		} else {
			target_val = -1;
		}

		// if (fold->state_iter%10000 == 0) {
		// 	cout << "target_val: " << target_val << endl;
		// }

		vector<double> local_s_input_vals;
		vector<double> local_state_vals;

		double predicted_score = -0.25;
		double scale_factor = 1.0;

		int explore_phase = EXPLORE_PHASE_NONE;

		ScopeHistory* scope_history = new ScopeHistory(scope);
		scope->explore_on_path_activate(flat_vals,
										local_s_input_vals,
										local_state_vals,
										predicted_score,
										scale_factor,
										explore_phase,
										scope_history);

		// if (fold->state_iter%10000 == 0) {
		// 	cout << "predicted_score: " << predicted_score << endl;
		// }

		vector<double> local_state_errors;
		scope->explore_on_path_backprop(local_state_errors,
										predicted_score,
										target_val,
										scale_factor,
										scope_history);
		delete scope_history;

		if (fold->state != -1) {
			break;
		}
	}

	Branch* inner_branch = inner_scope->branches[0];

	while (true) {
		vector<vector<double>> flat_vals;
		double target_val;

		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
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

			target_val = (double)((xor_1+xor_2)%2);
		} else {
			if (fold->state_iter%10000 == 0) {
				cout << "case #1" << endl;
			}

			flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
			int xor_1 = rand()%2;
			flat_vals.push_back(vector<double>{2*(double)xor_1-1});
			int xor_2 = rand()%2;
			flat_vals.push_back(vector<double>{2*(double)xor_2-1});

			target_val = (double)((xor_1+xor_2)%2);
		}
		flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});

		if (fold->state_iter%10000 == 0) {
			cout << "target_val: " << target_val << endl;
		}

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

		if (fold->state_iter%10000 == 0) {
			cout << "predicted_score: " << predicted_score << endl;
		}

		double next_predicted_score = predicted_score;

		scope->update_backprop(predicted_score,
							   next_predicted_score,
							   target_val,
							   scale_factor,
							   scope_history);
		delete scope_history;

		if (inner_branch->is_branch[1]) {
			break;
		}
	}

	{
		ofstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(scope->id) + ".txt");
		scope->save(scope_save_file);
		scope_save_file.close();
		cout << "outer: " << "saves/scope_" + to_string(scope->id) + ".txt" << endl;
	}

	delete scope;

	cout << "Done" << endl;
}