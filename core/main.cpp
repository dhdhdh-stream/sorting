#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "solution.h"

using namespace std;

default_random_engine generator;
bool global_debug_flag = false;
double global_sum_error = 0.0;

Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->init();

	int iter_index = 0;
	// while (true) {
	for (int i = 0; i < 2000; i++) {
		if (i%100 == 0) {
			cout << i << endl;
		}
		Problem problem;

		// if (iter_index > 200000 && rand()%2 == 0) {
		if (rand()%2 == 0) {
			bool is_pre_sorted;
			if (1.0 == problem.score_result()) {
				is_pre_sorted = true;
			} else {
				is_pre_sorted = false;
			}

			vector<double> local_s_input_vals;
			vector<double> local_state_vals;

			double predicted_score = 0.0;
			double scale_factor = 1.0;

			RunStatus run_status;

			ScopeHistory* scope_history = new ScopeHistory(solution->root);
			solution->root->explore_on_path_activate(problem,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 run_status,
													 scope_history);

			if (!run_status.exceeded_depth) {
				if (run_status.max_depth > solution->max_depth) {
					solution->max_depth = run_status.max_depth;

					if (solution->max_depth < 50) {
						solution->depth_limit = solution->max_depth + 10;
					} else {
						solution->depth_limit = (int)(1.2*(double)solution->max_depth);
					}
				}
			}

			double target_val;
			if (run_status.exceeded_depth) {
				target_val = -1.0;
			} else {
				target_val = problem.score_result();
			}
			double final_misguess = (target_val - predicted_score)*(target_val - predicted_score);

			bool explore_success = false;
			if (run_status.explore_phase == EXPLORE_PHASE_EXPLORE) {
				// if (target_val > run_status.existing_score) {
				if (rand()%2 == 0) {
					// if (run_status.predicted_misguess <= 0.0
					// 		|| (target_val-run_status.existing_score)/run_status.predicted_misguess > 1.0) {	// >75%
					if (true) {
						if (!is_pre_sorted) {
							cout << "not pre_sorted" << endl;
						}

						solution->root->explore_set(target_val,
													run_status.existing_score,
													run_status.predicted_misguess,
													scope_history);
						explore_success = true;
					}
				}

				solution->new_sequence_iter();
			} else if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
				vector<double> local_state_errors;
				double scale_factor_error = 0.0;
				solution->root->explore_on_path_backprop(local_state_errors,
														 predicted_score,
														 target_val,
														 final_misguess,
														 scale_factor,
														 scale_factor_error,
														 scope_history);
				explore_success = true;
			}

			if (!explore_success) {
				solution->root->explore_clear(scope_history);
			}

			delete scope_history;
		} else {
			vector<double> local_s_input_vals;
			vector<double> local_state_vals;

			double predicted_score = 0.0;
			double scale_factor = 1.0;

			RunStatus run_status;

			ScopeHistory* scope_history = new ScopeHistory(solution->root);
			solution->root->update_activate(problem,
											local_s_input_vals,
											local_state_vals,
											predicted_score,
											scale_factor,
											run_status,
											scope_history);

			if (!run_status.exceeded_depth) {
				if (run_status.max_depth > solution->max_depth) {
					solution->max_depth = run_status.max_depth;

					if (solution->max_depth < 50) {
						solution->depth_limit = solution->max_depth + 10;
					} else {
						solution->depth_limit = (int)(1.2*(double)solution->max_depth);
					}
				}
			}

			double target_val;
			if (run_status.exceeded_depth) {
				target_val = -1.0;
			} else {
				target_val = problem.score_result();
			}
			double final_misguess = (target_val - predicted_score)*(target_val - predicted_score);

			double scale_factor_error = 0.0;	// unused
			solution->root->update_backprop(predicted_score,
											target_val,
											final_misguess,
											scale_factor,
											scale_factor_error,
											scope_history);

			vector<Fold*> folds_to_delete;
			solution->root->update_increment(scope_history,
											 folds_to_delete);
			for (int f_index = 0; f_index < (int)folds_to_delete.size(); f_index++) {
				delete folds_to_delete[f_index];
			}

			delete scope_history;
		}

		iter_index++;
	}

	ofstream solution_save_file;
	solution_save_file.open("saves/solution.txt");
	solution->save(solution_save_file);
	solution_save_file.close();

	delete solution;

	{
		solution = new Solution();
		ifstream solution_save_file;
		solution_save_file.open("saves/solution.txt");
		solution->load(solution_save_file);
		solution_save_file.close();
	}

	for (int i = 0; i < 2000; i++) {
		if (i%100 == 0) {
			cout << i << endl;
		}
		Problem problem;

		// if (iter_index > 200000 && rand()%2 == 0) {
		if (rand()%2 == 0) {
			bool is_pre_sorted;
			if (1.0 == problem.score_result()) {
				is_pre_sorted = true;
			} else {
				is_pre_sorted = false;
			}

			vector<double> local_s_input_vals;
			vector<double> local_state_vals;

			double predicted_score = 0.0;
			double scale_factor = 1.0;

			RunStatus run_status;

			ScopeHistory* scope_history = new ScopeHistory(solution->root);
			solution->root->explore_on_path_activate(problem,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 run_status,
													 scope_history);

			if (!run_status.exceeded_depth) {
				if (run_status.max_depth > solution->max_depth) {
					solution->max_depth = run_status.max_depth;

					if (solution->max_depth < 50) {
						solution->depth_limit = solution->max_depth + 10;
					} else {
						solution->depth_limit = (int)(1.2*(double)solution->max_depth);
					}
				}
			}

			double target_val;
			if (run_status.exceeded_depth) {
				target_val = -1.0;
			} else {
				target_val = problem.score_result();
			}
			double final_misguess = (target_val - predicted_score)*(target_val - predicted_score);

			bool explore_success = false;
			if (run_status.explore_phase == EXPLORE_PHASE_EXPLORE) {
				// if (target_val > run_status.existing_score) {
				if (rand()%2 == 0) {
					// if (run_status.predicted_misguess <= 0.0
					// 		|| (target_val-run_status.existing_score)/run_status.predicted_misguess > 1.0) {	// >75%
					if (true) {
						if (!is_pre_sorted) {
							cout << "not pre_sorted" << endl;
						}

						solution->root->explore_set(target_val,
													run_status.existing_score,
													run_status.predicted_misguess,
													scope_history);
						explore_success = true;
					}
				}

				solution->new_sequence_iter();
			} else if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
				vector<double> local_state_errors;
				double scale_factor_error = 0.0;
				solution->root->explore_on_path_backprop(local_state_errors,
														 predicted_score,
														 target_val,
														 final_misguess,
														 scale_factor,
														 scale_factor_error,
														 scope_history);
				explore_success = true;
			}

			if (!explore_success) {
				solution->root->explore_clear(scope_history);
			}

			delete scope_history;
		} else {
			vector<double> local_s_input_vals;
			vector<double> local_state_vals;

			double predicted_score = 0.0;
			double scale_factor = 1.0;

			RunStatus run_status;

			ScopeHistory* scope_history = new ScopeHistory(solution->root);
			solution->root->update_activate(problem,
											local_s_input_vals,
											local_state_vals,
											predicted_score,
											scale_factor,
											run_status,
											scope_history);

			if (!run_status.exceeded_depth) {
				if (run_status.max_depth > solution->max_depth) {
					solution->max_depth = run_status.max_depth;

					if (solution->max_depth < 50) {
						solution->depth_limit = solution->max_depth + 10;
					} else {
						solution->depth_limit = (int)(1.2*(double)solution->max_depth);
					}
				}
			}

			double target_val;
			if (run_status.exceeded_depth) {
				target_val = -1.0;
			} else {
				target_val = problem.score_result();
			}
			double final_misguess = (target_val - predicted_score)*(target_val - predicted_score);

			double scale_factor_error = 0.0;	// unused
			solution->root->update_backprop(predicted_score,
											target_val,
											final_misguess,
											scale_factor,
											scale_factor_error,
											scope_history);

			vector<Fold*> folds_to_delete;
			solution->root->update_increment(scope_history,
											 folds_to_delete);
			for (int f_index = 0; f_index < (int)folds_to_delete.size(); f_index++) {
				delete folds_to_delete[f_index];
			}

			delete scope_history;
		}

		iter_index++;
	}

	delete solution;

	cout << "Done" << endl;
}
