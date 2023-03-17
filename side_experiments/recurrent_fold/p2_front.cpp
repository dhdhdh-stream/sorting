/**
 * 0: blank
 * 1: 1st val
 * 2: 2nd val
 * 3: blank
 * 4: switch
 * 5: blank
 * 6: shared val
 * 7: blank
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

	solution = new Solution();

	ActionNode* starting_node = (ActionNode*)solution->scopes[0]->nodes[0];

	Fold* fold = new Fold(vector<int>{0},
						  vector<int>{0},
						  0,
						  8,
						  vector<bool>(8, false),
						  vector<int>(8, -1),
						  0,
						  &starting_node->average_score,
						  &starting_node->score_variance,
						  &starting_node->average_misguess,
						  &starting_node->misguess_variance);
	starting_node->explore_scope_context = vector<int>{0};
	starting_node->explore_node_context = vector<int>{0};
	starting_node->explore_exit_depth = 0;
	starting_node->explore_next_node_id = -1;
	starting_node->explore_fold = fold;

	while (true) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

		if (rand()%5 == 0 && fold->state_iter < 200000) {
			flat_vals.push_back(vector<double>{0.0});	// extra for ACTION_START

			flat_vals.push_back(flat_vals[0]);
			int first_val = 0;
			flat_vals.push_back(vector<double>{(double)(first_val*2-1)});
			int second_val = 0;
			flat_vals.push_back(vector<double>{(double)(second_val*2-1)});
			flat_vals.push_back(vector<double>{0.0});
			int switch_val = 0;
			flat_vals.push_back(vector<double>{(double)(switch_val*2-1)});
			flat_vals.push_back(vector<double>{0.0});
			int shared_val = 0;
			flat_vals.push_back(vector<double>{(double)(shared_val*2-1)});
			flat_vals.push_back(vector<double>{0.0});

			if (switch_val == 0) {
				if ((first_val+shared_val)%2 == 0) {
					target_val = 1.0;
				} else {
					target_val = 0.0;
				}
			} else {
				target_val = -1.0;
			}
		} else {
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});	// extra for ACTION_START

			flat_vals.push_back(flat_vals[0]);
			int first_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(first_val*2-1)});
			int second_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(second_val*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
			int switch_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(switch_val*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
			int shared_val = rand()%2;
			flat_vals.push_back(vector<double>{(double)(shared_val*2-1)});
			flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

			if (switch_val == 0) {
				if ((first_val+shared_val)%2 == 0) {
					target_val = 1.0;
				} else {
					target_val = 0.0;
				}
			} else {
				target_val = -1.0;
			}
		}

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

		if (starting_node->explore_fold == NULL) {
			break;
		}
	}

	while (true) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});	// extra for ACTION_START

		flat_vals.push_back(flat_vals[0]);
		int first_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(first_val*2-1)});
		int second_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(second_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
		int switch_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(switch_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
		int shared_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(shared_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

		if (switch_val == 0) {
			if ((first_val+shared_val)%2 == 0) {
				target_val = 1.0;
			} else {
				target_val = 0.0;
			}
		} else {
			target_val = -1.0;
		}

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

		if (solution->scopes[0]->nodes[1]->type != NODE_TYPE_FOLD_SEQUENCE) {
			break;
		}
	}

	for (int i = 0; i < 10; i++) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});	// extra for ACTION_START

		flat_vals.push_back(flat_vals[0]);
		int first_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(first_val*2-1)});
		int second_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(second_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
		int switch_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(switch_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
		int shared_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(shared_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

		if (switch_val == 0) {
			if ((first_val+shared_val)%2 == 0) {
				target_val = 1.0;
			} else {
				target_val = 0.0;
			}
		} else {
			target_val = -1.0;
		}

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

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPLORE) {
			// add fold
		} else {
			cout << "target_val: " << target_val << endl;
			cout << "predicted_score: " << predicted_score << endl;

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

	ofstream solution_save_file;
	solution_save_file.open("saves/solution.txt");
	solution->save(solution_save_file);
	solution_save_file.close();

	delete solution;

	{
		ifstream solution_save_file;
		solution_save_file.open("saves/solution.txt");
		solution = new Solution(solution_save_file);
		solution_save_file.close();
	}

	for (int i = 0; i < 10; i++) {
		vector<vector<double>> flat_vals;
		double target_val = 0.0;

		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});	// extra for ACTION_START

		flat_vals.push_back(flat_vals[0]);
		int first_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(first_val*2-1)});
		int second_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(second_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
		int switch_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(switch_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});
		int shared_val = rand()%2;
		flat_vals.push_back(vector<double>{(double)(shared_val*2-1)});
		flat_vals.push_back(vector<double>{(double)(rand()%2*2-1)});

		if (switch_val == 0) {
			if ((first_val+shared_val)%2 == 0) {
				target_val = 1.0;
			} else {
				target_val = 0.0;
			}
		} else {
			target_val = -1.0;
		}

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

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPLORE) {
			// add fold
		} else {
			cout << "target_val: " << target_val << endl;
			cout << "predicted_score: " << predicted_score << endl;

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
