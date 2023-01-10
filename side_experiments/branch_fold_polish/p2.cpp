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

	vector<bool> is_existing(9, false);
	vector<Scope*> existing_actions(9, NULL);
	vector<int> obs_sizes(9, 1);

	double outer_average_score = 0.0;
	double outer_average_misguess = 0.0;

	Fold* fold = new Fold(9,
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

		outer_average_score = 0.999*outer_average_score + 0.001*target_val;

		double curr_misguess = (target_val - outer_average_score)*(target_val - outer_average_score);
		outer_average_misguess = 0.999*outer_average_misguess + 0.001*curr_misguess;

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

		if (explore_signal != EXPLORE_SIGNAL_NONE) {
			break;
		}
	}

	// fold->flat_to_fold();

	// deleting outer score networks for testing
	delete fold->starting_score_network;
	fold->starting_score_network = NULL;
	delete fold->combined_score_network;
	fold->combined_score_network = NULL;

	while (true) {
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

		double starting_score = 0.0;	// should be migrated score network, but simply use 0.0 for now

		vector<double> local_s_input_vals;
		vector<double> local_state_vals;

		double predicted_score = 0.0;
		double scale_factor = 1.0;

		FoldHistory* fold_history = new FoldHistory(fold);
		fold->update_activate(flat_vals,
							  starting_score,
							  local_s_input_vals,
							  local_state_vals,
							  predicted_score,
							  scale_factor,
							  fold_history);

		double next_predicted_score = predicted_score;
		fold->update_backprop(predicted_score,
							  next_predicted_score,
							  target_val,
							  scale_factor,
							  fold_history);
		delete fold_history;

		if (fold->state == STATE_DONE) {
			break;
		}
	}

	cout << "finished steps:" << endl;
	for (int f_index = 0; f_index < (int)fold->finished_steps.size(); f_index++) {
		cout << f_index << endl;
		cout << "fold->finished_steps[f_index]->inner_input_input_networks.size(): " << fold->finished_steps[f_index]->inner_input_input_networks.size() << endl;
		cout << "fold->finished_steps[f_index]->compress_num_layers: " << fold->finished_steps[f_index]->compress_num_layers << endl;
		cout << "fold->finished_steps[f_index]->compress_new_size: " << fold->finished_steps[f_index]->compress_new_size << endl;
		cout << "fold->finished_steps[f_index]->compress_original_size: " << fold->finished_steps[f_index]->compress_original_size << endl;
		cout << "fold->finished_steps[f_index]->input_networks.size(): " << fold->finished_steps[f_index]->input_networks.size() << endl;
	}
	cout << endl;

	int new_sequence_length;
	vector<bool> new_is_inner_scope;
	vector<Scope*> new_scopes;
	vector<int> new_obs_sizes;
	vector<vector<FoldNetwork*>> new_inner_input_networks;
	vector<vector<int>> new_inner_input_sizes;
	vector<double> new_scope_scale_mod;
	vector<int> new_step_types;
	vector<Branch*> new_branches;
	vector<Fold*> new_folds;
	vector<FoldNetwork*> new_score_networks;
	vector<double> new_average_scores;
	vector<double> new_average_misguesses;
	vector<double> new_average_inner_scope_impacts;
	vector<double> new_average_local_impacts;
	vector<double> new_average_inner_branch_impacts;
	vector<bool> new_active_compress;
	vector<int> new_compress_new_sizes;
	vector<FoldNetwork*> new_compress_networks;
	vector<int> new_compress_original_sizes;

	fold_to_path(fold->finished_steps,
				 new_sequence_length,
				 new_is_inner_scope,
				 new_scopes,
				 new_obs_sizes,
				 new_inner_input_networks,
				 new_inner_input_sizes,
				 new_scope_scale_mod,
				 new_step_types,
				 new_branches,
				 new_folds,
				 new_score_networks,
				 new_average_scores,
				 new_average_misguesses,
				 new_average_inner_scope_impacts,
				 new_average_local_impacts,
				 new_average_inner_branch_impacts,
				 new_active_compress,
				 new_compress_new_sizes,
				 new_compress_networks,
				 new_compress_original_sizes);

	delete fold;

	Scope* scope = new Scope(0,
							 0,
							 new_sequence_length,
							 new_is_inner_scope,
							 new_scopes,
							 new_obs_sizes,
							 new_inner_input_networks,
							 new_inner_input_sizes,
							 new_scope_scale_mod,
							 new_step_types,
							 new_branches,
							 new_folds,
							 new_score_networks,
							 new_average_scores,
							 new_average_misguesses,
							 new_average_inner_scope_impacts,
							 new_average_local_impacts,
							 new_average_inner_branch_impacts,
							 new_active_compress,
							 new_compress_new_sizes,
							 new_compress_networks,
							 new_compress_original_sizes);

	ofstream scope_save_file;
	scope_save_file.open("saves/scope_" + to_string(scope->id) + ".txt");
	scope->save(scope_save_file);
	scope_save_file.close();
	cout << "outer: " << "saves/scope_" + to_string(scope->id) + ".txt" << endl;

	// {
	// 	vector<vector<double>> flat_vals;
	// 	flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
	// 	int xor_1_1 = rand()%2;
	// 	flat_vals.push_back(vector<double>{2*(double)xor_1_1-1});
	// 	int xor_1_2 = rand()%2;
	// 	flat_vals.push_back(vector<double>{2*(double)xor_1_2-1});
	// 	flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});
	// 	int xor_2_1 = rand()%2;
	// 	flat_vals.push_back(vector<double>{2*(double)xor_2_1-1});
	// 	int xor_2_2 = rand()%2;
	// 	flat_vals.push_back(vector<double>{2*(double)xor_2_2-1});
	// 	flat_vals.push_back(vector<double>{2*(double)(rand()%2)-1});

	// 	double target_val = (double)((xor_1_1+xor_1_2)%2*2-1 + (xor_2_1+xor_2_2)%2*2-1);

	// 	vector<double> local_s_input_vals;
	// 	vector<double> local_state_vals;

	// 	double predicted_score = 0.0;
	// 	double scale_factor = 1.0;

	// 	ScopeHistory* scope_history = new ScopeHistory(scope);
	// 	scope->update_activate(flat_vals,
	// 						   local_s_input_vals,
	// 						   local_state_vals,
	// 						   predicted_score,
	// 						   scale_factor,
	// 						   scope_history);

	// 	double next_predicted_score = predicted_score;

	// 	scope->update_backprop(predicted_score,
	// 						   next_predicted_score,
	// 						   target_val,
	// 						   scale_factor,
	// 						   scope_history);
	// 	delete scope_history;
	// }

	delete scope;

	cout << "Done" << endl;
}