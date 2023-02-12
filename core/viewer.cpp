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
	// solution->init();
	ifstream solution_save_file;
	solution_save_file.open("saves/solution.txt");
	solution->load(solution_save_file);
	solution_save_file.close();

	Problem problem;
	problem.print();

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

	cout << "target_val: " << target_val << endl;

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

	delete solution;

	cout << "Done" << endl;
}
