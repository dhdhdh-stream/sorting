#include "action_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "solution.h"

using namespace std;

void ActionNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	problem->perform_action(this->action);

	vector<double> obs;
	vector<vector<double>> locations;
	problem->get_observations(obs, locations);
	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		run_helper.world_model[locations[o_index]] = obs[o_index];
	}

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().node_history[this] = {problem->get_location(), obs};
}
