#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "constants.h"
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

	int iter_index = 0;
	// chrono::steady_clock::time_point display_previous_time = chrono::steady_clock::now();
	while (true) {
		Problem problem;

		RunHelper run_helper;
		if (iter_index > 200000 && rand()%3 != 0) {
			run_helper.explore_phase = EXPLORE_PHASE_NONE;
		} else {
			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		}

		vector<double> input_vals(solution->scopes[0]->num_states, 0.0);
		vector<bool> inputs_initialized(solution->scopes[0]->num_states, false);
		double predicted_score = solution->average_score;
		double scale_factor = 1.0;
		double sum_impact = 0.0;
		vector<int> scope_context;
		vector<int> node_context;
		vector<ScopeHistory*> context_histories;
		
		// unused
		int early_exit_depth;
		int early_exit_node_id;
		FoldHistory* early_exit_fold_history;
		int explore_exit_depth;
		int explore_exit_node_id;
		FoldHistory* explore_exit_fold_history;

		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(problem,
									  input_vals,
									  inputs_initialized,
									  predicted_score,
									  scale_factor,
									  sum_impact,
									  scope_context,
									  node_context,
									  context_histories,
									  early_exit_depth,
									  early_exit_node_id,
									  early_exit_fold_history,
									  explore_exit_depth,
									  explore_exit_node_id,
									  explore_exit_fold_history,
									  run_helper,
									  root_history);

		if (run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		}

		double target_val;
		if (run_helper.exceeded_depth) {
			target_val = -1.0;
		} else {
			target_val = problem.score_result();
		}
		double final_misguess = (target_val - predicted_score)*(target_val - predicted_score);

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPLORE) {
			// add fold
		} else {
			if (!run_helper.exceeded_depth) {
				if (run_helper.max_depth > solution->max_depth) {
					solution->max_depth = run_helper.max_depth;

					if (solution->max_depth < 50) {
						solution->depth_limit = solution->max_depth + 10;
					} else {
						solution->depth_limit = (int)(1.2*(double)solution->max_depth);
					}
				}
			}

			solution->average_score = 0.9999*solution->average_score + 0.0001*target_val;

			vector<double> input_errors;
			solution->scopes[0]->backprop(input_errors,
										  inputs_initialized,
										  target_val,
										  final_misguess,
										  sum_impact,
										  predicted_score,
										  scale_factor,
										  run_helper,
										  root_history);
		}

		delete root_history;

		iter_index++;
		// if (iter_index%10000 == 0) {
		// 	chrono::steady_clock::time_point curr_time = chrono::steady_clock::now();
		// 	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(curr_time - display_previous_time);
		// 	if (time_span.count() > 120.0) {
		// 		ofstream display_file;
		// 		display_file.open("../display.txt");
		// 		solution->root->save_for_display(display_file);
		// 		display_file.close();

		// 		display_previous_time = curr_time;
		// 	}
		// }
	}

	delete solution;

	cout << "Done" << endl;
}
