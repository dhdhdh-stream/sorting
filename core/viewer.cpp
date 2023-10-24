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
	ifstream solution_save_file;
	solution_save_file.open("saves/solution.txt");
	solution->load(solution_save_file);
	solution_save_file.close();

	Scope* root = solution->scopes[0];

	{
		Problem problem;
		// Problem problem(vector<double>{3.0, 1.0});

		RunHelper run_helper;
		run_helper.phase = RUN_PHASE_UPDATE;

		vector<ContextLayer> context;
		context.push_back(ContextLayer());

		context.back().scope_id = 0;
		context.back().node_id = -1;

		vector<int> starting_node_ids{0};
		vector<map<int, StateStatus>> starting_input_state_vals;
		vector<map<int, StateStatus>> starting_local_state_vals;

		// unused
		int exit_depth = -1;
		int exit_node_id = -1;

		root->view_activate(starting_node_ids,
							starting_input_state_vals,
							starting_local_state_vals,
							problem,
							context,
							exit_depth,
							exit_node_id,
							run_helper);

		double target_val;
		if (!run_helper.exceeded_depth) {
			target_val = problem.score_result();
		} else {
			target_val = -1.0;
		}
		cout << "initial_world:";
		for (int p_index = 0; p_index < (int)problem.initial_world.size(); p_index++) {
			cout << " " << problem.initial_world[p_index];
		}
		cout << endl;
		cout << "ending_world:";
		for (int p_index = 0; p_index < (int)problem.current_world.size(); p_index++) {
			cout << " " << problem.current_world[p_index];
		}
		cout << endl;
		cout << "target_val: " << target_val << endl;
	}

	delete solution;

	cout << "Done" << endl;
}
