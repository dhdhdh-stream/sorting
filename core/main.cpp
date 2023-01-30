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

	// int seed = (unsigned)time(NULL);
	int seed = 1674449097;
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->init();

	// while (true) {
	for (int i = 0; i < 10000; i++) {
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

			if (run_status.explore_phase == EXPLORE_PHASE_EXPLORE) {
				// !run_status.exceeded_depth
				// if (target_val > explore_status.existing_score) {
				if (rand()%10 == 0) {
					solution->root->explore_set(scope_history);
				}
			} else if (run_status.explore_phase == EXPLORE_PHASE_FLAT) {
				vector<double> local_state_errors;
				solution->root->explore_on_path_backprop(local_state_errors,
														 predicted_score,
														 target_val,
														 scale_factor,
														 scope_history);
			}
			// can be EXPLORE_PHASE_NONE if early exit or edge case if root is entirely replaced

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
			delete scope_history;
		}
	}

	delete solution;

	cout << "Done" << endl;
}
