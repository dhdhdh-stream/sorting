#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "solution.h"
#include "world_model.h"

using namespace std;

void ActionNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
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

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().node_history[this] = {problem->get_location(), obs};
}
