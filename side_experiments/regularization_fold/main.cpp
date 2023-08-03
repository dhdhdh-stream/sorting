#include <chrono>
#include <iostream>
#include <thread>
#include <random>

using namespace std;

default_random_engine generator;
bool global_debug_flag = false;
double global_sum_error = 0.0;

Solution* solution;

void explore_impacts_helper(vector<ActionNode*>& potential_nodes,
							vector<double>& impacts,
							double& temp_scale_factor,
							ScopeHistory* scope_history) {
	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				potential_nodes.push_back((ActionNode*)action_node_history->node);
				impacts.push_back(temp_scale_factor*action_node_history->score_network_output);
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				temp_scale_factor *= scope_node->scope_scale_mod->weight;

				explore_impacts_helper(potential_nodes,
									   impacts,
									   temp_scale_factor,
									   scope_node_history->inner_scope_history);

				temp_scale_factor /= scope_node->scope_scale_mod->weight;
			}
		}
	}
}

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	// ifstream solution_save_file;
	// solution_save_file.open("saves/solution.txt");
	// solution = new Solution(solution_save_file);
	// solution_save_file.close();

	int iter_index = 0;
	chrono::steady_clock::time_point display_previous_time = chrono::steady_clock::now();
	while (true) {
		// Problem problem;
		vector<double> flat_vals;

		RunHelper run_helper;
		if (iter_index > 100000 && rand()%3 != 0) {
			run_helper.explore_phase = EXPLORE_PHASE_NONE;
		} else {
			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;

			if (rand()%10 == 0) {
				run_helper.can_random_iter = true;
			} else {
				run_helper.can_random_iter = false;
			}
		}

		vector<ForwardContextLayer> context;
		context.push_back(ForwardContextLayer());

		context.back().scope_id = 0;
		context.back().node_id = -1;

		vector<double> inner_state_vals(solution->scopes[0]->num_states, 0.0);
		context.back().state_vals = &inner_state_vals;
		context.back().states_initialized = vector<bool>(solution->scopes[0]->num_states, true);
		// minor optimization to initialize to true to prevent updating last_seen_vals for starting scope

		ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
		context.back().scope_history = root_history;

		vector<int> starting_node_ids{0};
		vector<vector<double>*> starting_state_vals;
		vector<vector<bool>> starting_states_initialized;

		int exit_depth;
		int exit_node_id;

		solution->scopes[0]->activate(starting_node_ids,
									  starting_state_vals,
									  starting_states_initialized,
									  flat_vals,
									  context,
									  exit_depth,
									  exit_node_id,
									  run_helper,
									  root_history);

		if (run_helper.explore_phase == EXPLORE_PHASE_NONE) {
			if (!run_helper.explore_seen) {
				vector<ActionNode*> potential_nodes;
				vector<double> impacts;

				double temp_scale_factor = 1.0;

				explore_impacts_helper(potential_nodes,
									   impacts,
									   temp_scale_factor,
									   root_history);

				double sum_impacts = 0.0;
				for (int p_index = 0; p_index < (int)potential_nodes.size(); p_index++) {
					sum_impacts += abs(impacts[p_index]);
				}

				double rand_val = sum_impacts*randuni();
				for (int p_index = 0; p_index < (int)potential_nodes.size(); p_index++) {
					rand_val -= abs(impacts[p_index]);
					if (rand_val <= 0.0) {
						potential_nodes[p_index]->is_explore = true;
						potential_nodes[p_index]->explore_curr_try = 0;
						potential_nodes[p_index]->explore_best_surprise = numeric_limits<double>::lowest();
						break;
					}
				}
			}

			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		}

		// run_helper.target_val = problem.score_result();
		run_helper.final_misguess = (run_helper.target_val - run_helper.predicted_score)*(run_helper.target_val - run_helper.predicted_score);

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPLORE) {
			double surprise = run_helper.target_val - run_helper.explore_experiment->seed_start_predicted_score;

			if (surprise > run_helper.explore_node->explore_best_surprise) {
				if (run_helper.explore_node->explore_best_experiment != NULL) {
					for (int a_index = 0; a_index < run_helper.explore_node->explore_best_experiment->num_steps) {
						if (run_helper.explore_node->explore_best_experiment->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
							delete run_helper.explore_node->explore_best_experiment->sequences[a_index];
						}
					}
					delete run_helper.explore_node->explore_best_experiment->seed_context_history;
					delete run_helper.explore_node->explore_best_experiment;
				}

				run_helper.explore_node->explore_best_surprise = surprise;
				run_helper.explore_node->explore_best_experiment = run_helper.explore_experiment;
				run_helper.explore_node->explore_best_experiment->seed_target_val = run_helper.target_val;
			}

			run_helper.explore_node->explore_curr_try++;
			if (run_helper.explore_curr_try >= EXPLORE_TARGET_TRIES) {
				action_node->experiment = action_node->explore_best_experiment;
				action_node->explore_best_experiment = NULL;
			}
		} else {
			if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE) {
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

				solution->average_score = 0.9999*solution->average_score + 0.0001*run_helper.target_val;
				double curr_score_variance = (solution->average_score - run_helper.target_val)*(solution->average_score - run_helper.target_val);
				solution->score_variance = 0.9999*solution->score_variance + 0.0001*curr_score_variance;
				solution->average_misguess = 0.9999*solution->average_misguess + 0.0001*run_helper.final_misguess;
				double curr_misguess_variance = (solution->average_misguess - run_helper.final_misguess)*(solution->average_misguess - run_helper.final_misguess);
				solution->misguess_variance = 0.9999*solution->misguess_variance + 0.0001*curr_misguess_variance;
				solution->misguess_standard_deviation = sqrt(solution->misguess_variance);
			}

			vector<BackwardContextLayer> context;
			context.push_back(BackwardContextLayer());

			vector<double> inner_state_errors(solution->scopes[0]->num_states, 0.0);
			context.back().state_errors = &inner_state_errors;

			vector<int> starting_node_ids{0};
			vector<vector<double>*> starting_state_errors;
			double inner_scale_factor_error = 0.0;

			solution->scopes[0]->backprop(starting_node_ids,
										  starting_state_errors,
										  context,
										  inner_scale_factor_error,
										  run_helper,
										  root_history);
		}

		delete root_history;

		// iter_index++;
		// if (iter_index%10000 == 0) {
		// 	chrono::steady_clock::time_point curr_time = chrono::steady_clock::now();
		// 	chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(curr_time - display_previous_time);
		// 	if (time_span.count() > 120.0) {
		// 		ofstream display_file;
		// 		display_file.open("../display.txt");
		// 		solution->save_for_display(display_file);
		// 		display_file.close();

		// 		display_previous_time = curr_time;
		// 	}
		// }
	}

	delete solution;

	cout << "Done" << endl;
}
