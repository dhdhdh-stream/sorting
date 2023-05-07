#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "action_node.h"
#include "constants.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

default_random_engine generator;
bool global_debug_flag = false;
double global_sum_error = 0.0;

Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	// int seed = (unsigned)time(NULL);
	int seed = 1683405762;
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();

	int iter_index = 0;
	chrono::steady_clock::time_point display_previous_time = chrono::steady_clock::now();
	while (true) {
		Problem problem;

		RunHelper run_helper;
		if (iter_index > 100000 && rand()%3 != 0) {
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
		double final_diff = target_val - predicted_score;
		double final_misguess = (target_val - predicted_score)*(target_val - predicted_score);

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPLORE) {
			double curr_surprise = target_val - run_helper.explore_seed_start_predicted_score;
			if (solution->scopes[run_helper.explore_scope_id]->nodes[run_helper.explore_node_id]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)solution->scopes[run_helper.explore_scope_id]->nodes[run_helper.explore_node_id];

				action_node->explore_curr_try++;
				if (action_node->explore_curr_try == 1 || curr_surprise > action_node->best_explore_surprise) {
					action_node->best_explore_surprise = curr_surprise;
					action_node->best_explore_scope_context = run_helper.explore_scope_context;
					action_node->best_explore_node_context = run_helper.explore_node_context;
					action_node->best_explore_is_loop = run_helper.explore_is_loop;
					action_node->best_explore_is_inner_scope = run_helper.explore_is_inner_scope;
					action_node->best_explore_existing_scope_ids = run_helper.explore_existing_scope_ids;
					action_node->best_explore_actions = run_helper.explore_actions;

					if (action_node->best_explore_seed_outer_context_history != NULL) {
						delete action_node->best_explore_seed_outer_context_history;
					}

					if (run_helper.explore_is_loop) {
						action_node->best_explore_exit_depth = -1;
						action_node->best_explore_next_node_id = -1;
						action_node->best_explore_seed_start_predicted_score = 0.0;
						action_node->best_explore_seed_start_scale_factor = 0.0;
						action_node->best_explore_seed_state_vals_snapshot.clear();
						action_node->best_explore_seed_outer_context_history = NULL;
					} else {
						action_node->best_explore_exit_depth = run_helper.explore_exit_depth;
						action_node->best_explore_next_node_id = run_helper.explore_next_node_id;
						action_node->best_explore_seed_start_predicted_score = run_helper.explore_seed_start_predicted_score;
						action_node->best_explore_seed_start_scale_factor = run_helper.explore_seed_start_scale_factor;
						action_node->best_explore_seed_state_vals_snapshot = run_helper.explore_seed_state_vals_snapshot;
						action_node->best_explore_seed_outer_context_history = run_helper.explore_seed_outer_context_history;
						run_helper.explore_seed_outer_context_history = NULL;
					}
				}

				if (action_node->explore_curr_try >= action_node->explore_target_tries) {
					if (action_node->best_explore_is_loop) {
						cout << "loop" << endl;
						cout << "action_node->explore_target_tries: " << action_node->explore_target_tries << endl;
						cout << "action_node->best_explore_surprise: " << action_node->best_explore_surprise << endl;
						cout << "actions:";
						for (int f_index = 0; f_index < (int)action_node->best_explore_actions.size(); f_index++) {
							if (action_node->best_explore_is_inner_scope[f_index]) {
								cout << " S" << action_node->best_explore_existing_scope_ids[f_index];
							} else {
								cout << " " << action_node->best_explore_actions[f_index].to_string();
							}
						}
						cout << endl;
						LoopFold* loop_fold = new LoopFold(action_node->best_explore_scope_context,
														   action_node->best_explore_node_context,
														   action_node->best_explore_is_inner_scope,
														   action_node->best_explore_existing_scope_ids,
														   action_node->best_explore_actions,
														   &action_node->average_score,
														   &action_node->score_variance,
														   &action_node->average_misguess,
														   &action_node->misguess_variance);

						action_node->explore_scope_context = action_node->best_explore_scope_context;
						action_node->explore_node_context = action_node->best_explore_node_context;
						action_node->explore_exit_depth = -1;
						action_node->explore_next_node_id = -1;
						action_node->explore_loop_fold = loop_fold;
					} else {
						cout << "path" << endl;
						cout << "action_node->explore_target_tries: " << action_node->explore_target_tries << endl;
						cout << "action_node->best_explore_surprise: " << action_node->best_explore_surprise << endl;
						cout << "actions:";
						for (int f_index = 0; f_index < (int)action_node->best_explore_actions.size(); f_index++) {
							if (action_node->best_explore_is_inner_scope[f_index]) {
								cout << " S" << action_node->best_explore_existing_scope_ids[f_index];
							} else {
								cout << " " << action_node->best_explore_actions[f_index].to_string();
							}
						}
						cout << endl;
						Fold* fold = new Fold(action_node->best_explore_scope_context,
											  action_node->best_explore_node_context,
											  action_node->best_explore_exit_depth,
											  action_node->best_explore_is_inner_scope,
											  action_node->best_explore_existing_scope_ids,
											  action_node->best_explore_actions,
											  &action_node->average_score,
											  &action_node->score_variance,
											  &action_node->average_misguess,
											  &action_node->misguess_variance,
											  action_node->best_explore_seed_start_predicted_score,
											  action_node->best_explore_seed_start_scale_factor,
											  action_node->best_explore_seed_state_vals_snapshot,
											  action_node->best_explore_seed_outer_context_history,
											  target_val);

						action_node->explore_scope_context = action_node->best_explore_scope_context;
						action_node->explore_node_context = action_node->best_explore_node_context;
						action_node->explore_exit_depth = action_node->best_explore_exit_depth;
						action_node->explore_next_node_id = action_node->best_explore_next_node_id;
						action_node->explore_fold = fold;
					}

					action_node->explore_curr_try = 0;
					action_node->explore_target_tries = 1;
					int rand_scale = rand()%4;
					for (int i = 0; i < rand_scale; i++) {
						action_node->explore_target_tries *= 10;
					}
					action_node->best_explore_surprise = numeric_limits<double>::lowest();
					action_node->best_explore_seed_outer_context_history = NULL;
				}
			} else {
				// solution->scopes[run_helper.explore_scope_id]->nodes[run_helper.explore_node_id]->type == NODE_TYPE_ACTION
				ScopeNode* scope_node = (ScopeNode*)solution->scopes[run_helper.explore_scope_id]->nodes[run_helper.explore_node_id];

				scope_node->explore_curr_try++;
				if (curr_surprise > scope_node->best_explore_surprise) {
					scope_node->best_explore_surprise = curr_surprise;
					scope_node->best_explore_scope_context = run_helper.explore_scope_context;
					scope_node->best_explore_node_context = run_helper.explore_node_context;
					scope_node->best_explore_is_loop = run_helper.explore_is_loop;
					scope_node->best_explore_is_inner_scope = run_helper.explore_is_inner_scope;
					scope_node->best_explore_existing_scope_ids = run_helper.explore_existing_scope_ids;
					scope_node->best_explore_actions = run_helper.explore_actions;

					if (scope_node->best_explore_seed_outer_context_history != NULL) {
						delete scope_node->best_explore_seed_outer_context_history;
					}

					if (run_helper.explore_is_loop) {
						scope_node->best_explore_exit_depth = -1;
						scope_node->best_explore_next_node_id = -1;
						scope_node->best_explore_seed_start_predicted_score = 0.0;
						scope_node->best_explore_seed_start_scale_factor = 0.0;
						scope_node->best_explore_seed_state_vals_snapshot.clear();
						scope_node->best_explore_seed_outer_context_history = NULL;
					} else {
						scope_node->best_explore_exit_depth = run_helper.explore_exit_depth;
						scope_node->best_explore_next_node_id = run_helper.explore_next_node_id;
						scope_node->best_explore_seed_start_predicted_score = run_helper.explore_seed_start_predicted_score;
						scope_node->best_explore_seed_start_scale_factor = run_helper.explore_seed_start_scale_factor;
						scope_node->best_explore_seed_state_vals_snapshot = run_helper.explore_seed_state_vals_snapshot;
						scope_node->best_explore_seed_outer_context_history = run_helper.explore_seed_outer_context_history;
						run_helper.explore_seed_outer_context_history = NULL;
					}
				}

				if (scope_node->explore_curr_try >= scope_node->explore_target_tries) {
					if (scope_node->best_explore_is_loop) {
						cout << "loop" << endl;
						cout << "scope_node->explore_target_tries: " << scope_node->explore_target_tries << endl;
						cout << "scope_node->best_explore_surprise: " << scope_node->best_explore_surprise << endl;
						cout << "actions:";
						for (int f_index = 0; f_index < (int)scope_node->best_explore_actions.size(); f_index++) {
							if (scope_node->best_explore_is_inner_scope[f_index]) {
								cout << " S" << scope_node->best_explore_existing_scope_ids[f_index];
							} else {
								cout << " " << scope_node->best_explore_actions[f_index].to_string();
							}
						}
						cout << endl;
						LoopFold* loop_fold = new LoopFold(scope_node->best_explore_scope_context,
														   scope_node->best_explore_node_context,
														   scope_node->best_explore_is_inner_scope,
														   scope_node->best_explore_existing_scope_ids,
														   scope_node->best_explore_actions,
														   &scope_node->average_score,
														   &scope_node->score_variance,
														   &scope_node->average_misguess,
														   &scope_node->misguess_variance);

						scope_node->explore_scope_context = scope_node->best_explore_scope_context;
						scope_node->explore_node_context = scope_node->best_explore_node_context;
						scope_node->explore_exit_depth = -1;
						scope_node->explore_next_node_id = -1;
						scope_node->explore_loop_fold = loop_fold;
					} else {
						cout << "path" << endl;
						cout << "scope_node->explore_target_tries: " << scope_node->explore_target_tries << endl;
						cout << "scope_node->best_explore_surprise: " << scope_node->best_explore_surprise << endl;
						cout << "actions:";
						for (int f_index = 0; f_index < (int)scope_node->best_explore_actions.size(); f_index++) {
							if (scope_node->best_explore_is_inner_scope[f_index]) {
								cout << " S" << scope_node->best_explore_existing_scope_ids[f_index];
							} else {
								cout << " " << scope_node->best_explore_actions[f_index].to_string();
							}
						}
						cout << endl;
						Fold* fold = new Fold(scope_node->best_explore_scope_context,
											  scope_node->best_explore_node_context,
											  scope_node->best_explore_exit_depth,
											  scope_node->best_explore_is_inner_scope,
											  scope_node->best_explore_existing_scope_ids,
											  scope_node->best_explore_actions,
											  &scope_node->average_score,
											  &scope_node->score_variance,
											  &scope_node->average_misguess,
											  &scope_node->misguess_variance,
											  scope_node->best_explore_seed_start_predicted_score,
											  scope_node->best_explore_seed_start_scale_factor,
											  scope_node->best_explore_seed_state_vals_snapshot,
											  scope_node->best_explore_seed_outer_context_history,
											  target_val);

						scope_node->explore_scope_context = scope_node->best_explore_scope_context;
						scope_node->explore_node_context = scope_node->best_explore_node_context;
						scope_node->explore_exit_depth = scope_node->best_explore_exit_depth;
						scope_node->explore_next_node_id = scope_node->best_explore_next_node_id;
						scope_node->explore_fold = fold;
					}

					scope_node->explore_curr_try = 0;
					scope_node->explore_target_tries = 1;
					int rand_scale = rand()%4;
					for (int i = 0; i < rand_scale; i++) {
						scope_node->explore_target_tries *= 10;
					}
					scope_node->best_explore_surprise = numeric_limits<double>::lowest();
					scope_node->best_explore_seed_outer_context_history = NULL;
				}
			}

			if (run_helper.explore_seed_outer_context_history != NULL) {
				delete run_helper.explore_seed_outer_context_history;
			}
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

			vector<double> input_errors(solution->scopes[0]->num_states, 0.0);
			double scale_factor_error = 0.0;	// unused
			solution->scopes[0]->backprop(input_errors,
										  inputs_initialized,
										  target_val,
										  final_diff,
										  final_misguess,
										  sum_impact,
										  predicted_score,
										  scale_factor,
										  scale_factor_error,
										  run_helper,
										  root_history);
		}

		delete root_history;

		iter_index++;
		if (iter_index%10000 == 0) {
			chrono::steady_clock::time_point curr_time = chrono::steady_clock::now();
			chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(curr_time - display_previous_time);
			if (time_span.count() > 120.0) {
				ofstream display_file;
				display_file.open("../display.txt");
				solution->save_for_display(display_file);
				display_file.close();

				display_previous_time = curr_time;
			}
		}
	}

	delete solution;

	cout << "Done" << endl;
}
