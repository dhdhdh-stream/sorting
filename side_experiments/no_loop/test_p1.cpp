/**
 * 0: blank
 * 1: switch
 * 2: blank
 * 3: xor
 * 4: xor
 * 5: blank
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "context_layer.h"
#include "globals.h"
#include "helpers.h"
#include "run_helper.h"
#include "scope.h"
#include "solution.h"
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

	Scope* root = solution->scopes[0];
	for (int n_index = 0; n_index < 6; n_index++) {
		((ActionNode*)root->nodes[n_index])->next_node_id = n_index+1;

		ActionNode* action_node = new ActionNode();
		action_node->id = n_index+1;
		root->nodes.push_back(action_node);
	}
	((ActionNode*)root->nodes.back())->next_node_id = -1;

	int iter_index = 0;
	// chrono::steady_clock::time_point display_previous_time = chrono::steady_clock::now();
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
		// if (iter_index > 500000 && rand()%3 != 0) {
		if (false) {
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
					   flat_vals,
					   context,
					   exit_depth,
					   exit_node_id,
					   run_helper,
					   root_history);

		double target_val;
		if (switch_val == 0) {
			target_val = -1;
		} else {
			target_val = (double)((xor_1+xor_2)%2);
		}

		if (run_helper.phase == RUN_PHASE_EXPLORE) {
			if (run_helper.experiments_seen_counts.size() == 0) {
				create_branch_experiment(root_history);
			} else {
				if (run_helper.branch_experiment_history != NULL) {
					BranchExperiment* experiment = run_helper.branch_experiment_history->experiment;
					experiment->backprop(target_val,
										 run_helper.branch_experiment_history);
				}
			}
		} else {
			for (int h_index = 0; h_index < (int)run_helper.scope_histories.size(); h_index++) {
				Scope* scope = run_helper.scope_histories[h_index]->scope;
				scope->update_backprop(target_val,
									   run_helper.scope_histories[h_index]);
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
		}

		delete root_history;

		iter_index++;
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

		if (solution->states.size() > 0) {
			break;
		}
	}

	{
		ofstream solution_save_file;
		solution_save_file.open("saves/solution.txt");
		solution->save(solution_save_file);
		solution_save_file.close();
	}

	delete solution;

	{
		solution = new Solution();
		ifstream solution_save_file;
		solution_save_file.open("saves/solution.txt");
		solution->load(solution_save_file);
		solution_save_file.close();

		root = solution->scopes[0];
	}

	global_debug_flag = true;

	for (int iter_index = 0; iter_index < 10; iter_index++) {
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
					   flat_vals,
					   context,
					   exit_depth,
					   exit_node_id,
					   run_helper,
					   root_history);

		double target_val;
		if (switch_val == 0) {
			target_val = -1;
		} else {
			target_val = (double)((xor_1+xor_2)%2);
		}

		cout << "target_val: " << target_val << endl;

		delete root_history;
	}

	delete solution;

	cout << "Done" << endl;
}
