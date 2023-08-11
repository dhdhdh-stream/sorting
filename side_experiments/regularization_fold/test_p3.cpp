#include <chrono>
#include <iostream>
#include <thread>
#include <random>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "context_layer.h"
#include "globals.h"
#include "run_helper.h"
#include "scope.h"
#include "score_network.h"
#include "sequence.h"
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

	ifstream solution_save_file;
	solution_save_file.open("saves/solution.txt");
	solution = new Solution(solution_save_file);
	solution_save_file.close();

	// for (int n_index = 0; n_index < (int)solution->scopes[0]->nodes.size(); n_index++) {
	// 	cout << n_index << ": " << solution->scopes[0]->nodes[n_index]->type << endl;
	// }
	// for (int n_index = 0; n_index < (int)solution->scopes[1]->nodes.size(); n_index++) {
	// 	cout << n_index << ": " << solution->scopes[1]->nodes[n_index]->type << endl;
	// }
	// cout << "num_states: " << solution->scopes[1]->num_states << endl;

	ActionNode* explore_node = (ActionNode*)solution->scopes[1]->nodes[2];

	Sequence* sequence = new Sequence(
		vector<Scope*>{solution->scopes[0]},
		vector<int>{2, 8},
		vector<int>(5, SEQUENCE_INPUT_TYPE_LOCAL),
		vector<int>{1, 1, 1, 1, 1},
		vector<int>{0, 1, 2, 3, 4},
		vector<int>{0, 0, 0, 0, 0},
		vector<int>{0, 1, 2, 3, 4},
		vector<int>(5, -1),
		vector<int>(5, -1),
		vector<int>(5, -1),
		vector<bool>(5, false),
		vector<Transformation>(5, Transformation()),
		vector<vector<int>>{vector<int>{2}});

	BranchExperiment* branch_experiment = new BranchExperiment(
		vector<int>{0, 1},
		vector<int>{2, 2},
		1,
		vector<int>{BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE},
		vector<Sequence*>{sequence},
		1,
		-1,
		-1.0,
		1.0,
		vector<double>(),
		new ScopeHistory(solution->scopes[0]),
		explore_node->score_network);
	branch_experiment->explore_transform();

	explore_node->is_explore = true;
	explore_node->experiment = branch_experiment;

	// for (int iter_index = 0; iter_index < 10; iter_index++) {
	// 	vector<double> flat_vals;
	// 	flat_vals.push_back(2*(double)(rand()%2)-1);
	// 	flat_vals.push_back(flat_vals[0]);	// copy for ACTION_START
	// 	int switch_val = rand()%2;
	// 	flat_vals.push_back(2*(double)switch_val-1);
	// 	flat_vals.push_back(2*(double)(rand()%2)-1);
	// 	flat_vals.push_back(2*(double)(rand()%2)-1);
	// 	flat_vals.push_back(2*(double)(rand()%2)-1);
	// 	flat_vals.push_back(2*(double)(rand()%2)-1);

	// 	vector<double> flat_vals_snapshot = flat_vals;

	// 	RunHelper run_helper;
	// 	run_helper.predicted_score = solution->average_score;
	// 	run_helper.scale_factor = 1.0;
	// 	run_helper.explore_phase = EXPLORE_PHASE_NONE;
	// 	if (rand()%10 == 0) {
	// 		run_helper.can_random_iter = true;
	// 	} else {
	// 		run_helper.can_random_iter = false;
	// 	}

	// 	vector<ForwardContextLayer> context;
	// 	context.push_back(ForwardContextLayer());

	// 	context.back().scope_id = 0;
	// 	context.back().node_id = -1;

	// 	vector<double> inner_state_vals(solution->scopes[0]->num_states, 0.0);
	// 	context.back().state_vals = &inner_state_vals;
	// 	context.back().states_initialized = vector<bool>(solution->scopes[0]->num_states, true);
	// 	// minor optimization to initialize to true to prevent updating last_seen_vals for starting scope

	// 	ScopeHistory* root_history = new ScopeHistory(solution->scopes[0]);
	// 	context.back().scope_history = root_history;

	// 	vector<int> starting_node_ids{0};
	// 	vector<vector<double>*> starting_state_vals;
	// 	vector<vector<bool>> starting_states_initialized;

	// 	int exit_depth;
	// 	int exit_node_id;

	// 	solution->scopes[0]->activate(starting_node_ids,
	// 								  starting_state_vals,
	// 								  starting_states_initialized,
	// 								  flat_vals,
	// 								  context,
	// 								  exit_depth,
	// 								  exit_node_id,
	// 								  run_helper,
	// 								  root_history);

	// 	cout << "run_helper.explore_phase: " << run_helper.explore_phase << endl;
	// 	cout << "switch_val: " << switch_val << endl;
	// 	cout << "flat_vals.size(): " << flat_vals.size() << endl;
	// 	cout << "run_helper.predicted_score: " << run_helper.predicted_score << endl;

	// 	delete root_history;
	// }

	int iter_index = 0;
	while (true) {
		vector<double> flat_vals;
		flat_vals.push_back(2*(double)(rand()%2)-1);
		flat_vals.push_back(flat_vals[0]);	// copy for ACTION_START
		int switch_val = rand()%2;
		flat_vals.push_back(2*(double)switch_val-1);
		flat_vals.push_back(2*(double)(rand()%2)-1);
		int xor_1 = rand()%2;
		flat_vals.push_back(2*(double)xor_1-1);
		int xor_2 = rand()%2;
		flat_vals.push_back(2*(double)xor_2-1);
		flat_vals.push_back(2*(double)(rand()%2)-1);

		RunHelper run_helper;
		run_helper.predicted_score = solution->average_score;
		run_helper.scale_factor = 1.0;
		if (rand()%3 != 0) {
			run_helper.explore_phase = EXPLORE_PHASE_NONE;
		} else {
			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		}
		if (rand()%10 == 0) {
			run_helper.can_random_iter = true;
		} else {
			run_helper.can_random_iter = false;
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
			// don't explore

			run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		}

		if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
				|| run_helper.explore_phase == EXPLORE_PHASE_CLEAN
				|| run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
			if (flat_vals.size() == 1) {
				if (switch_val == 0) {
					run_helper.target_val = 1 + 2*(double)((xor_1+xor_2)%2);
				} else {
					run_helper.target_val = 0;
				}
			} else {
				if (switch_val == 0) {
					run_helper.target_val = 0;
				} else {
					run_helper.target_val = 1 + 2*(double)((xor_1+xor_2)%2);
				}
			}
		} else {
			if (flat_vals.size() == 1) {
				if (switch_val == 0) {
					run_helper.target_val = (double)((xor_1+xor_2)%2);
				} else {
					run_helper.target_val = -1;
				}
			} else {
				if (switch_val == 0) {
					run_helper.target_val = -1;
				} else {
					run_helper.target_val = (double)((xor_1+xor_2)%2);
				}
			}
		}
		run_helper.final_misguess = (run_helper.target_val - run_helper.predicted_score)*(run_helper.target_val - run_helper.predicted_score);

		{
			if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
					|| run_helper.explore_phase == EXPLORE_PHASE_WRAPUP) {
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
			} else {
				run_helper.new_state_errors = vector<double>(NUM_NEW_STATES, 0.0);
				run_helper.backprop_is_pre_experiment = false;
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

		if (explore_node->experiment == NULL) {
			break;
		}
		iter_index++;
	}

	delete solution;

	cout << "Done" << endl;
}
