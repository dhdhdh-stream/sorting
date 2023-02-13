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
	// ifstream solution_save_file;
	// solution_save_file.open("saves/solution.txt");
	// solution->load(solution_save_file);
	// solution_save_file.close();

	int iter_index = 0;
	chrono::steady_clock::time_point display_previous_time = chrono::steady_clock::now();
	while (true) {
		Problem problem;

		if (iter_index > 200000 && rand()%2 == 0) {
			vector<double> local_s_input_vals;
			vector<double> local_state_vals;

			double predicted_score = solution->average_score;
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

			if (run_status.explore_phase == EXPLORE_PHASE_EXPLORE) {
				double score_standard_deviation = sqrt(run_status.score_variance);
				if ((target_val-run_status.existing_score)/score_standard_deviation > 1.0) {	// >75%
					solution->root->explore_set(target_val,
												run_status.existing_score,
												scope_history);
				}
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
			}

			delete scope_history;
		} else {
			vector<double> local_s_input_vals;
			vector<double> local_state_vals;

			double predicted_score = solution->average_score;
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

			solution->average_score = 0.9999*solution->average_score + 0.0001*target_val;

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
		if (iter_index%10000 == 0) {
			chrono::steady_clock::time_point curr_time = chrono::steady_clock::now();
			chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(curr_time - display_previous_time);
			if (time_span.count() > 120.0) {
				ofstream display_file;
				display_file.open("../display.txt");
				solution->root->save_for_display(display_file);
				display_file.close();

				display_previous_time = curr_time;
			}
		}
	}

	delete solution;

	cout << "Done" << endl;
}
