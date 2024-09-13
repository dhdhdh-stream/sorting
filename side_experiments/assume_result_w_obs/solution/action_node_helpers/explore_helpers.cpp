#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "world_model.h"

using namespace std;

void ActionNode::explore_activate(Problem* problem,
								  RunHelper& run_helper) {
	problem->perform_action(this->action);

	vector<double> obs;
	vector<vector<int>> locations;
	problem->get_observations(obs, locations);
	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		if (run_helper.world_model == NULL) {
			run_helper.world_model = new WorldModel(locations[o_index],
													obs[o_index]);
		} else {
			run_helper.world_model->update(locations[o_index],
										   obs[o_index]);
		}
	}

	run_helper.num_actions++;
}
