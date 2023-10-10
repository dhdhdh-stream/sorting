#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "context_layer.h"
#include "globals.h"
#include "helpers.h"
#include "run_helper.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_status.h"

using namespace std;

default_random_engine generator;

bool global_debug_flag = false;

Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	solution = new Solution();
	solution->init();
	// ifstream solution_save_file;
	// solution_save_file.open("saves/solution.txt");
	// solution->load(solution_save_file);
	// solution_save_file.close();

	Scope* root = solution->scopes[0];

	ActionNode* explore_node = (ActionNode*)root->nodes[0];
	vector<int> experiment_scope_context{0};
	vector<int> experiment_node_context{0};
	explore_node->experiment = new BranchExperiment(experiment_scope_context,
													experiment_node_context);
	explore_node->experiment->state = BRANCH_EXPERIMENT_STATE_TRAIN;
	explore_node->experiment->state_iter = 0;
	explore_node->experiment->best_step_types.push_back(STEP_TYPE_ACTION);
	explore_node->experiment->best_actions.push_back(new ActionNode());
	explore_node->experiment->best_actions.back()->action = Action(ACTION_RIGHT);
	explore_node->experiment->best_sequences.push_back(NULL);
	explore_node->experiment->best_step_types.push_back(STEP_TYPE_ACTION);
	explore_node->experiment->best_actions.push_back(new ActionNode());
	explore_node->experiment->best_actions.back()->action = Action(ACTION_RIGHT);
	explore_node->experiment->best_sequences.push_back(NULL);
	explore_node->experiment->best_exit_depth = 0;
	explore_node->experiment->best_exit_node_id = -1;

	int iter_index = 0;
	chrono::steady_clock::time_point display_previous_time = chrono::steady_clock::now();
	while (true) {
		Problem problem;

		RunHelper run_helper;
		if (solution->states.size() > 0 && rand()%3 != 0) {
			run_helper.phase = RUN_PHASE_EXPLORE;
		} else {
			run_helper.phase = RUN_PHASE_UPDATE;
		}

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope_id = 0;
		context.back().node_id = -1;

		ScopeHistory* root_history = new ScopeHistory(root);
		context.back().scope_history = root_history;

		vector<int> starting_node_ids{0};
		vector<map<int, StateStatus>> starting_input_state_vals;
		vector<map<int, StateStatus>> starting_local_state_vals;

		// unused
		int exit_depth = -1;
		int exit_node_id = -1;

		root->activate(starting_node_ids,
					   starting_input_state_vals,
					   starting_local_state_vals,
					   problem,
					   context,
					   exit_depth,
					   exit_node_id,
					   run_helper,
					   root_history);

		double target_val;
		if (!run_helper.exceeded_depth) {
			if (run_helper.max_depth > solution->max_depth) {
				solution->max_depth = run_helper.max_depth;

				if (solution->max_depth < 50) {
					solution->depth_limit = solution->max_depth + 10;
				} else {
					solution->depth_limit = (int)(1.2*(double)solution->max_depth);
				}
			}

			target_val = problem.score_result();
		} else {
			target_val = -1.0;
		}

		if (run_helper.phase == RUN_PHASE_EXPLORE) {
			if (run_helper.experiments_seen_counts.size() == 0) {
				create_branch_experiment(root_history);
			} else {
				if (run_helper.selected_branch_experiment != NULL) {
					run_helper.selected_branch_experiment->unhook();

					if (run_helper.branch_experiment_history != NULL) {
						BranchExperiment* experiment = run_helper.branch_experiment_history->experiment;
						experiment->backprop(target_val,
											 run_helper.branch_experiment_history);

						if (experiment->state == BRANCH_EXPERIMENT_STATE_DONE) {
							Scope* starting_scope = solution->scopes[experiment->scope_context.back()];
							AbstractNode* starting_node = starting_scope->nodes[experiment->node_context.back()];
							if (starting_node->type == NODE_TYPE_ACTION) {
								ActionNode* action_node = (ActionNode*)starting_node;
								action_node->experiment = NULL;
							} else if (starting_node->type == NODE_TYPE_SCOPE) {
								ScopeNode* scope_node = (ScopeNode*)starting_node;
								scope_node->experiment = NULL;
							} else {
								BranchNode* branch_node = (BranchNode*)starting_node;
								branch_node->experiment = NULL;
							}
							delete experiment;
						}
					}
				}
			}
		} else {
			// run_helper.phase == RUN_PHASE_UPDATE

			set<State*> states_to_remove;
			for (int h_index = 0; h_index < (int)run_helper.scope_histories.size(); h_index++) {
				Scope* scope = run_helper.scope_histories[h_index]->scope;
				scope->update_backprop(target_val,
									   run_helper.scope_histories[h_index],
									   states_to_remove);
			}

			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				BranchExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.999 * experiment->average_remaining_experiments_from_start
					+ 0.001 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
			}

			for (map<BranchExperiment*, int>::iterator it = run_helper.experiments_seen_counts.begin();
					it != run_helper.experiments_seen_counts.end(); it++) {
				BranchExperiment* experiment = it->first;
				experiment->average_instances_per_run = 0.999 * experiment->average_instances_per_run + 0.001 * it->second;
			}

			for (set<State*>::iterator it = states_to_remove.begin(); it != states_to_remove.end(); it++) {
				delete *it;
			}
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

		if (iter_index%1000000 == 0) {
			cout << iter_index << endl;
			cout << "solution->states.size(): " << solution->states.size() << endl;
			cout << "root->average_score: " << root->average_score << endl;
			cout << "root->average_misguess: " << root->average_misguess << endl;
		}
	}

	delete solution;

	cout << "Done" << endl;
}
