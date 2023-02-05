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

	while (true) {
		Problem problem;

		if (rand()%2 == 0) {
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

			bool explore_success = false;
			if (run_status.explore_phase == EXPLORE_PHASE_EXPLORE) {
				// !run_status.exceeded_depth
				if (target_val > run_status.existing_score) {
					double score_standard_deviation = sqrt(run_status.score_variance);
					double t_value = (target_val-run_status.existing_score) / score_standard_deviation;
					if (t_value > 1.0) {	// >75%
						cout << "EXPLORE_SET" << endl;
						cout << "target_val: " << target_val << endl;
						cout << "run_status.existing_score: " << run_status.existing_score << endl;
						cout << "t_value: " << t_value << endl;
						solution->root->explore_set(scope_history);
						explore_success = true;
					}
				}
			} else if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
				vector<double> local_state_errors;
				solution->root->explore_on_path_backprop(local_state_errors,
														 predicted_score,
														 target_val,
														 scale_factor,
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

			double next_predicted_score = predicted_score;

			double scale_factor_error = 0.0;	// unused
			solution->root->update_backprop(predicted_score,
											next_predicted_score,
											target_val,
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
	}

	delete solution;

	cout << "Done" << endl;
}
