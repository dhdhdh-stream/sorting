#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ActionNode::verify_activate(AbstractNode*& curr_node,
								 Problem& problem,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper) {
	problem.perform_action(this->action);
	double obs_snapshot = problem.get_observation();

	vector<double> state_snapshots(this->state_is_local.size(), 0.0);
	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		if (this->state_is_local[n_index]) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
			if (it == context.back().local_state_vals.end()) {
				it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
			}
			FullNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
			if (this->state_obs_indexes[n_index] == -1) {
				state_network->activate(obs_snapshot,
										it->second);
			} else {
				state_network->activate(state_snapshots[this->state_obs_indexes[n_index]],
										it->second);
			}
			state_snapshots[n_index] = it->second.val;
		} else {
			map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
			if (it != context.back().input_state_vals.end()) {
				FullNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				if (this->state_obs_indexes[n_index] == -1) {
					state_network->activate(obs_snapshot,
											it->second);
				} else {
					state_network->activate(state_snapshots[this->state_obs_indexes[n_index]],
											it->second);
				}
				state_snapshots[n_index] = it->second.val;
			}
		}
	}

	curr_node = this->next_node;
}
