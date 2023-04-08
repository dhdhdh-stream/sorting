/**
 * 0: blank
 * 1: loop
 * 2: blank
 */

#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "run_helper.h"
#include "scope.h"
#include "scope_node.h"
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
	cout << "Seed: " << seed << endl;

	ifstream solution_save_file;
	solution_save_file.open("saves/solution.txt");
	solution = new Solution(solution_save_file);
	solution_save_file.close();

	ScopeNode* starting_node = (ScopeNode*)solution->scopes[0]->nodes[4];

	LoopFold* loop_fold = new LoopFold(vector<int>{0},
									   vector<int>{4},
									   1,
									   vector<bool>(1, true),
									   vector<int>(1, 1),
									   &starting_node->average_score,
									   &starting_node->score_variance,
									   &starting_node->average_misguess,
									   &starting_node->misguess_variance);
	starting_node->explore_scope_context = vector<int>{0};
	starting_node->explore_node_context = vector<int>{4};
	starting_node->explore_loop_fold = loop_fold;

	while (true) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});	// extra for ACTION_START

		flat_vals.push_back(flat_vals[0]);

		int halt_val = rand()%8;
		for (int i_index = 0; i_index < 10; i_index++) {
			int first_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(first_val*2-1)});

			if (halt_val == i_index) {
				flat_vals.push_back(vector<double>{1.0});
			} else {
				flat_vals.push_back(vector<double>{0.0});
			}

			int second_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(second_val*2-1)});

			if (halt_val == i_index) {
				if ((first_val+second_val)%2 == 0) {
					target_val = 1.0;
				} else {
					target_val = 0.0;
				}
			}
		}

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

		vector<double> input_vals;
		double predicted_score = solution->average_score;
		double scale_factor = 1.0;
		double sum_impact = 0.0;
		vector<int> scope_context;
		vector<int> node_context;
		vector<ScopeHistory*> context_histories;
		int early_exit_depth;
		int early_exit_node_id;
		FoldHistory* early_exit_fold_history;
		int explore_exit_depth;
		int explore_exit_node_id;
		FoldHistory* explore_exit_fold_history;
		RunHelper run_helper;
		if (rand()%3 == 0) {
			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		} else {
			run_helper.explore_phase = EXPLORE_PHASE_NONE;
		}
		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(input_vals,
									  flat_vals,
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

		int correct_length = 33 - 3 - 3 - 3*halt_val;
		if ((int)flat_vals.size() != correct_length) {
			target_val = -1.0;
		}

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPLORE) {
			// add fold
		} else {
			solution->average_score = 0.9999*solution->average_score + 0.0001*target_val;
			double final_misguess = abs(target_val - predicted_score);

			vector<double> input_errors;
			solution->scopes[0]->backprop(input_errors,
										  target_val,
										  final_misguess,
										  sum_impact,
										  predicted_score,
										  scale_factor,
										  run_helper,
										  root_history);
		}

		delete root_history;

		if (starting_node->explore_loop_fold == NULL) {
			break;
		}
	}

	while (true) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});	// extra for ACTION_START

		flat_vals.push_back(flat_vals[0]);

		int halt_val = rand()%8;
		for (int i_index = 0; i_index < 10; i_index++) {
			int first_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(first_val*2-1)});

			if (halt_val == i_index) {
				flat_vals.push_back(vector<double>{1.0});
			} else {
				flat_vals.push_back(vector<double>{0.0});
			}

			int second_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(second_val*2-1)});

			if (halt_val == i_index) {
				if ((first_val+second_val)%2 == 0) {
					target_val = 1.0;
				} else {
					target_val = 0.0;
				}
			}
		}

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

		vector<double> input_vals;
		double predicted_score = solution->average_score;
		double scale_factor = 1.0;
		double sum_impact = 0.0;
		vector<int> scope_context;
		vector<int> node_context;
		vector<ScopeHistory*> context_histories;
		int early_exit_depth;
		int early_exit_node_id;
		FoldHistory* early_exit_fold_history;
		int explore_exit_depth;
		int explore_exit_node_id;
		FoldHistory* explore_exit_fold_history;
		RunHelper run_helper;
		if (rand()%3 == 0) {
			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		} else {
			run_helper.explore_phase = EXPLORE_PHASE_NONE;
		}
		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(input_vals,
									  flat_vals,
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

		int correct_length = 33 - 3 - 3 - 3*halt_val;
		if ((int)flat_vals.size() != correct_length) {
			target_val = -1.0;
		}

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPLORE) {
			// add fold
		} else {
			solution->average_score = 0.9999*solution->average_score + 0.0001*target_val;
			double final_misguess = abs(target_val - predicted_score);

			vector<double> input_errors;
			solution->scopes[0]->backprop(input_errors,
										  target_val,
										  final_misguess,
										  sum_impact,
										  predicted_score,
										  scale_factor,
										  run_helper,
										  root_history);
		}

		delete root_history;

		if (solution->scopes[0]->nodes[6]->type != NODE_TYPE_LOOP_FOLD) {
			break;
		}
	}

	for (int i = 0; i < 50; i++) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});	// extra for ACTION_START

		flat_vals.push_back(flat_vals[0]);

		int halt_val = rand()%8;
		for (int i_index = 0; i_index < 10; i_index++) {
			int first_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(first_val*2-1)});

			if (halt_val == i_index) {
				flat_vals.push_back(vector<double>{1.0});
			} else {
				flat_vals.push_back(vector<double>{0.0});
			}

			int second_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(second_val*2-1)});

			if (halt_val == i_index) {
				if ((first_val+second_val)%2 == 0) {
					target_val = 1.0;
				} else {
					target_val = 0.0;
				}
			}
		}

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

		vector<double> input_vals;
		double predicted_score = solution->average_score;
		double scale_factor = 1.0;
		double sum_impact = 0.0;
		vector<int> scope_context;
		vector<int> node_context;
		vector<ScopeHistory*> context_histories;
		int early_exit_depth;
		int early_exit_node_id;
		FoldHistory* early_exit_fold_history;
		int explore_exit_depth;
		int explore_exit_node_id;
		FoldHistory* explore_exit_fold_history;
		RunHelper run_helper;
		if (rand()%3 == 0) {
			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		} else {
			run_helper.explore_phase = EXPLORE_PHASE_NONE;
		}
		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		solution->scopes[0]->activate(input_vals,
									  flat_vals,
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

		int correct_length = 33 - 3 - 3 - 3*halt_val;
		if ((int)flat_vals.size() != correct_length) {
			target_val = -1.0;
		}

		cout << "halt_val: " << halt_val << endl;
		cout << "target_val: " << target_val << endl;
		cout << "predicted_score: " << predicted_score << endl;
		cout << endl;

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPLORE) {
			// add fold
		} else {
			solution->average_score = 0.9999*solution->average_score + 0.0001*target_val;
			double final_misguess = abs(target_val - predicted_score);

			vector<double> input_errors;
			solution->scopes[0]->backprop(input_errors,
										  target_val,
										  final_misguess,
										  sum_impact,
										  predicted_score,
										  scale_factor,
										  run_helper,
										  root_history);
		}

		delete root_history;
	}

	delete solution;

	cout << "Done" << endl;
}
