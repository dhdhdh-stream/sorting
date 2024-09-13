#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "new_action_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "world_model.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
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
	if (run_helper.experiments_seen_order.size() == 0) {
		map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.find({this, false});
		if (it == run_helper.nodes_seen.end()) {
			run_helper.nodes_seen[{this, false}] = 1;
		} else {
			it->second++;
		}
	} else if (run_helper.experiment_histories.size() == 1
			&& run_helper.experiment_histories.back()->experiment == this->parent->new_action_experiment) {
		context.back().nodes_seen.push_back({this, false});
	}

	context.back().node_history[this] = {problem->get_location(), obs};

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper);
		if (is_selected) {
			return;
		}
	}
}
