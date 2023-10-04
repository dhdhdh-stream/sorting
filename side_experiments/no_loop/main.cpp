#include <chrono>
#include <iostream>
#include <thread>
#include <random>

using namespace std;

default_random_engine generator;

Solution* solution;

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

	Scope* root = solution->scopes[0];

	int iter_index = 0;
	// chrono::steady_clock::time_point display_previous_time = chrono::steady_clock::now();
	while (true) {
		// Problem problem;
		vector<double> flat_vals;

		RunHelper run_helper;
		if (iter_index > 500000 && rand()%3 != 0) {
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
		int exit_depth;
		int exit_node_id;

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
		// target_val = problem.score_result();

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
					+ 0.001 * (run_helper.experiments_seen_order.size()-1 - e_index);
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
	}

	delete solution;

	cout << "Done" << endl;
}
