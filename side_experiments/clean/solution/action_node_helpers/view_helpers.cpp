#include "action_node.h"

#include <iostream>

#include "constants.h"
#include "state_network.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ActionNode::view_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   vector<ContextLayer>& context,
							   int& exit_depth,
							   AbstractNode*& exit_node,
							   RunHelper& run_helper) {
	problem->perform_action(this->action);
	if (this->action.move != ACTION_NOOP) {
		run_helper.num_actions++;
	}
	double obs_snapshot = problem->get_observation();

	cout << "action node #" << this->id << endl;
	cout << "action: " << this->action.move << endl;
	cout << "obs: " << obs_snapshot << endl;

	vector<double> state_snapshots(this->state_is_local.size(), 0.0);
	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		if (this->state_is_local[n_index]) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
			if (it == context.back().local_state_vals.end()) {
				it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
			if (this->state_obs_indexes[n_index] == -1) {
				state_network->activate(obs_snapshot,
										it->second);
			} else {
				state_network->activate(state_snapshots[this->state_obs_indexes[n_index]],
										it->second);
			}
			state_snapshots[n_index] = it->second.val;
			cout << "local state #" << this->state_indexes[n_index] << ": " << it->second.val << endl;
		} else {
			map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
			if (it != context.back().input_state_vals.end()) {
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				if (this->state_obs_indexes[n_index] == -1) {
					state_network->activate(obs_snapshot,
											it->second);
				} else {
					state_network->activate(state_snapshots[this->state_obs_indexes[n_index]],
											it->second);
				}
				state_snapshots[n_index] = it->second.val;
				cout << "input state #" << this->state_indexes[n_index] << ": " << it->second.val << endl;
			}
		}
	}

	cout << endl;

	curr_node = this->next_node;
}
