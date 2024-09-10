#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ActionNode::explore_activate(Problem* problem,
								  RunHelper& run_helper) {
	problem->perform_action(this->action);

	vector<double> obs;
	vector<vector<double>> locations;
	problem->get_observations(obs, locations);
	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		run_helper.world_model[locations[o_index]] = obs[o_index];
	}

	run_helper.num_actions++;
}
