/**
 * - too sharp to hit
 * 
 * - to hit sharp targets, need world modeling?
 *   - learn effect of actions (without worrying about decision making within)
 *     - (this happens at multiple scales)
 *   - then monte carlo tree search actions to come up with a plan
 */

/**
 * 0: blank
 * 1: loop iters
 * 2: blank
 * - loop
 *   - 0: xor
 *   - 1: blank
 *   - 2: xor
 * 6: blank
 */

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
#include "loop_experiment.h"
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

	// cout << "num_states: " << solution->scopes[1]->num_states << endl;

	ActionNode* explore_node = (ActionNode*)solution->scopes[1]->nodes[5];

	Sequence* sequence = new Sequence(
		vector<Scope*>{solution->scopes[1]},
		vector<int>{3},
		vector<int>(4, SEQUENCE_INPUT_TYPE_LOCAL),
		vector<int>{0, 0, 0, 0},
		vector<int>{0, 1, 2, 3},
		vector<int>{0, 0, 0, 0},
		vector<int>{0, 1, 2, 3},
		vector<int>(4, -1),
		vector<int>(4, -1),
		vector<int>(4, -1),
		vector<bool>(4, false),
		vector<Transformation>(4, Transformation()),
		vector<vector<int>>{vector<int>{3, 4, 5}});

	LoopExperiment* loop_experiment = new LoopExperiment(
		vector<int>{1},
		vector<int>{5},
		sequence,
		explore_node->misguess_network);

	sequence->experiment = loop_experiment;
	sequence->step_index = 0;

	loop_experiment->explore_transform();

	explore_node->is_explore = true;
	explore_node->experiment = loop_experiment;

	int iter_index = 0;
	while (true) {
		vector<double> flat_vals;
		flat_vals.push_back(2*(double)(rand()%2)-1);
		flat_vals.push_back(flat_vals[0]);	// copy for ACTION_START
		int loop_iters = 1+rand()%10;
		flat_vals.push_back(loop_iters);
		flat_vals.push_back(2*(double)(rand()%2)-1);

		double correct_loop_target_val;
		for (int i = 0; i < 10; i++) {
			int xor_1 = rand()%2;
			flat_vals.push_back(2*(double)xor_1-1);
			flat_vals.push_back(2*(double)(rand()%2)-1);
			int xor_2 = rand()%2;
			flat_vals.push_back(2*(double)xor_2-1);

			if (i == loop_iters) {
				correct_loop_target_val = (double)((xor_1+xor_2)%2);
			}
		}

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
			run_helper.can_train_loops = true;
		} else {
			run_helper.can_train_loops = false;
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

		if ((int)flat_vals.size() == 3*(10 - loop_iters)) {
			run_helper.target_val = correct_loop_target_val;
		} else {
			run_helper.target_val = -1;
		}
		run_helper.final_misguess = (run_helper.target_val - run_helper.predicted_score)*(run_helper.target_val - run_helper.predicted_score);

		if (run_helper.explore_phase == EXPLORE_PHASE_MEASURE) {
			run_helper.experiment->new_average_score += run_helper.target_val;
			run_helper.experiment->new_average_misguess += run_helper.final_misguess;

			run_helper.experiment->state_iter++;
			if (run_helper.experiment->state_iter == 10000) {
				LoopExperiment* loop_experiment = (LoopExperiment*)run_helper.experiment;
				loop_experiment->experiment_transform();

				if (run_helper.experiment->state == EXPERIMENT_STATE_DONE) {
					Scope* explore_scope = solution->scopes[run_helper.experiment->scope_context.back()];
					ActionNode* action_node = (ActionNode*)explore_scope->nodes[run_helper.experiment->node_context.back()];

					action_node->is_explore = false;

					delete action_node->experiment;
					action_node->experiment = NULL;
				}
			}
		} else {
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

	for (int iter_index = 0; iter_index < 50; iter_index++) {
		vector<double> flat_vals;
		flat_vals.push_back(2*(double)(rand()%2)-1);
		flat_vals.push_back(flat_vals[0]);	// copy for ACTION_START
		int loop_iters = 1+rand()%10;
		flat_vals.push_back(loop_iters);
		flat_vals.push_back(2*(double)(rand()%2)-1);

		double correct_loop_target_val;
		for (int i = 0; i < 10; i++) {
			int xor_1 = rand()%2;
			flat_vals.push_back(2*(double)xor_1-1);
			flat_vals.push_back(2*(double)(rand()%2)-1);
			int xor_2 = rand()%2;
			flat_vals.push_back(2*(double)xor_2-1);

			if (i == loop_iters) {
				correct_loop_target_val = (double)((xor_1+xor_2)%2);
			}
		}

		flat_vals.push_back(2*(double)(rand()%2)-1);

		RunHelper run_helper;
		run_helper.predicted_score = solution->average_score;
		run_helper.scale_factor = 1.0;
		run_helper.explore_phase = EXPLORE_PHASE_UPDATE;
		run_helper.can_train_loops = false;

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

		if ((int)flat_vals.size() == 3*(10 - loop_iters)) {
			run_helper.target_val = correct_loop_target_val;
		} else {
			run_helper.target_val = -1;
		}

		cout << iter_index << endl;
		cout << "loop_iters: " << loop_iters << endl;
		cout << "flat_vals.size(): " << flat_vals.size() << endl;
		cout << "run_helper.target_val: " << run_helper.target_val << endl;
		cout << "run_helper.predicted_score: " << run_helper.predicted_score << endl;

		delete root_history;
	}

	delete solution;

	cout << "Done" << endl;
}
