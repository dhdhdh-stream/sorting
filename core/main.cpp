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

			ExploreStatus explore_status;

			ScopeHistory* scope_history = new ScopeHistory(solution->root);
			solution->root->explore_on_path_activate(problem,
													 local_s_input_vals,
													 local_state_vals,
													 predicted_score,
													 scale_factor,
													 explore_status,
													 scope_history);

			double target_val = problem.score_result();
			if (explore_status.explore_phase == EXPLORE_PHASE_EXPLORE) {
				if (target_val > explore_status.existing_score) {
					solution->root->explore_set(scope_history);
				}
			} else {
				// explore_status.explore_phase == EXPLORE_PHASE_FLAT
				vector<double> local_state_errors;
				solution->root->explore_on_path_backprop(local_state_errors,
														 predicted_score,
														 target_val,
														 scale_factor,
														 scope_history);
			}

			delete scope_history;
		} else {
			vector<double> local_s_input_vals;
			vector<double> local_state_vals;

			double predicted_score = 0.0;
			double scale_factor = 1.0;

			ScopeHistory* scope_history = new ScopeHistory(solution->root);
			solution->root->update_activate(problem,
											local_s_input_vals,
											local_state_vals,
											predicted_score,
											scale_factor,
											scope_history);

			double target_val = problem.score_result();

			double next_predicted_score = predicted_score;

			solution->root->update_backprop(predicted_score,
											next_predicted_score,
											target_val,
											scale_factor,
											scope_history);
			delete scope_history;
		}
	}

	delete solution;

	cout << "Done" << endl;
}
