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
#include "sequence.h"
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

	// ActionNode* explore_node = (ActionNode*)root->nodes[0];
	// vector<int> experiment_scope_context{0};
	// vector<int> experiment_node_context{0};
	// explore_node->experiment = new BranchExperiment(experiment_scope_context,
	// 												experiment_node_context);
	// explore_node->experiment->best_step_types.push_back(STEP_TYPE_ACTION);
	// explore_node->experiment->best_actions.push_back(new ActionNode());
	// explore_node->experiment->best_actions.back()->action = Action(ACTION_RIGHT);
	// explore_node->experiment->best_sequences.push_back(NULL);
	// explore_node->experiment->best_step_types.push_back(STEP_TYPE_ACTION);
	// explore_node->experiment->best_actions.push_back(new ActionNode());
	// explore_node->experiment->best_actions.back()->action = Action(ACTION_LEFT);
	// explore_node->experiment->best_sequences.push_back(NULL);
	// explore_node->experiment->best_exit_depth = 0;
	// explore_node->experiment->best_exit_node_id = -1;

	// ActionNode* explore_node = (ActionNode*)root->nodes[2];
	// vector<int> experiment_scope_context{0};
	// vector<int> experiment_node_context{2};
	// explore_node->experiment = new BranchExperiment(experiment_scope_context,
	// 												experiment_node_context);
	// explore_node->experiment->best_step_types.push_back(STEP_TYPE_ACTION);
	// explore_node->experiment->best_actions.push_back(new ActionNode());
	// explore_node->experiment->best_actions.back()->action = Action(ACTION_SWAP);
	// explore_node->experiment->best_sequences.push_back(NULL);
	// explore_node->experiment->best_exit_depth = 0;
	// explore_node->experiment->best_exit_node_id = 3;

	// // ActionNode* explore_node = (ActionNode*)root->nodes[3];
	// ActionNode* explore_node = (ActionNode*)root->nodes[6];
	// vector<int> experiment_scope_context{0};
	// // vector<int> experiment_node_context{3};
	// vector<int> experiment_node_context{6};
	// explore_node->experiment = new BranchExperiment(experiment_scope_context,
	// 												experiment_node_context);
	// explore_node->experiment->best_step_types.push_back(STEP_TYPE_SEQUENCE);
	// explore_node->experiment->best_actions.push_back(NULL);
	// Scope* explore_scope = new Scope();
	// explore_scope->id = solution->scope_counter;
	// solution->scope_counter++;
	// explore_scope->num_input_states = 0;
	// explore_scope->num_local_states = 0;
	// ScopeNode* new_explore_scope_node = new ScopeNode();
	// new_explore_scope_node->id = 0;
	// new_explore_scope_node->inner_scope = solution->scopes[0];
	// new_explore_scope_node->starting_node_ids = vector<int>{0};
	// new_explore_scope_node->next_node_id = -1;
	// explore_scope->nodes.push_back(new_explore_scope_node);
	// explore_scope->average_score = root->average_score;
	// explore_scope->score_variance = root->score_variance;
	// explore_scope->average_misguess = root->average_misguess;
	// explore_scope->misguess_variance = root->misguess_variance;
	// Sequence* explore_sequence = new Sequence();
	// explore_sequence->scope = explore_scope;
	// explore_node->experiment->best_sequences.push_back(explore_sequence);
	// explore_node->experiment->best_exit_depth = 0;
	// explore_node->experiment->best_exit_node_id = -1;
	// explore_node->experiment->recursion_protection = true;

	uniform_int_distribution<int> explore_distribution(0, 2);
	while (true) {
		// update
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			Problem problem;

			RunHelper run_helper;
			run_helper.phase = RUN_PHASE_UPDATE;

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
				target_val = problem.score_result();
			} else {
				target_val = -1.0;
			}

			solution->average_score = 0.999*solution->average_score + 0.001*target_val;

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

			for (int h_index = 0; h_index < (int)run_helper.scope_histories.size(); h_index++) {
				Scope* scope = run_helper.scope_histories[h_index]->scope;
				scope->update_histories(target_val,
										run_helper.scope_histories[h_index]);
			}

			delete root_history;
		}

		for (map<int, Scope*>::iterator it = solution->scopes.begin();
				it != solution->scopes.end(); it++) {
			it->second->update();
		}

		while (true) {
			Problem problem;

			RunHelper run_helper;
			run_helper.phase = RUN_PHASE_EXPLORE;

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
				target_val = problem.score_result();
			} else {
				target_val = -1.0;
			}

			bool updated = false;
			if (run_helper.experiments_seen_counts.size() == 0) {
				create_branch_experiment(root_history);
			} else {
				if (run_helper.selected_branch_experiment != NULL) {
					run_helper.selected_branch_experiment->unhook();

					for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
						BranchExperiment* experiment = run_helper.experiments_seen_order[e_index];
						experiment->average_remaining_experiments_from_start =
							0.9 * experiment->average_remaining_experiments_from_start
							+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
								+ run_helper.selected_branch_experiment->average_remaining_experiments_from_start);
					}

					run_helper.selected_branch_experiment->average_instances_per_run
						= 0.9 * run_helper.selected_branch_experiment->average_instances_per_run
							+ 0.1 * (run_helper.selected_branch_experiment_count + 1);
					/**
					 * - set to 1 more than instances seen as exit might have skipped instances
					 */

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

							updated = true;
						}
					}
				} else {
					for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
						BranchExperiment* experiment = run_helper.experiments_seen_order[e_index];
						experiment->average_remaining_experiments_from_start =
							0.9 * experiment->average_remaining_experiments_from_start
							+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
					}

					for (map<BranchExperiment*, int>::iterator it = run_helper.experiments_seen_counts.begin();
							it != run_helper.experiments_seen_counts.end(); it++) {
						BranchExperiment* experiment = it->first;
						experiment->average_instances_per_run = 0.9 * experiment->average_instances_per_run + 0.1 * it->second;
					}
				}
			}

			delete root_history;

			if (updated) {
				break;
			}
		}

		cout << "solution->states.size(): " << solution->states.size() << endl;
		cout << "solution->average_score: " << solution->average_score << endl;
	}

	delete solution;

	cout << "Done" << endl;
}
