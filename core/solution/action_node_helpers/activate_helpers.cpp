#include "action_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ActionNode::activate(int& curr_node_id,
						  Problem& problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  int& exit_node_id,
						  RunHelper& run_helper,
						  vector<AbstractNodeHistory*>& node_histories) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	node_histories.push_back(history);

	problem.perform_action(this->action);
	history->obs_snapshot = problem.get_observation();

	if (run_helper.phase == RUN_PHASE_EXPLORE) {
		for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
			if (this->state_is_local[n_index]) {
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
				if (it == context.back().local_state_vals.end()) {
					it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				StateStatus state_impact;
				state_network->activate(history->obs_snapshot,
										it->second,
										state_impact);
				history->state_indexes.push_back(n_index);
				history->state_impacts.push_back(state_impact);
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
				if (it != context.back().input_state_vals.end()) {
					StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
					StateStatus state_impact;
					state_network->activate(history->obs_snapshot,
											it->second,
											state_impact);
					history->state_indexes.push_back(n_index);
					history->state_impacts.push_back(state_impact);
				}
			}
		}
	} else {
		for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
			if (this->state_is_local[n_index]) {
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
				if (it == context.back().local_state_vals.end()) {
					it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				state_network->activate(history->obs_snapshot,
										it->second);
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
				if (it != context.back().input_state_vals.end()) {
					StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
					state_network->activate(history->obs_snapshot,
											it->second);
				}
			}
		}
	}

	for (int n_index = 0; n_index < (int)this->experiment_hook_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_hook_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_hook_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_hook_state_scope_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()+c_index].scope_id
						|| this->experiment_hook_state_node_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<int, StateStatus>::iterator it = context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()]
				.experiment_state_vals.find(this->experiment_hook_state_indexes[n_index]);
			if (it == context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()].experiment_state_vals.end()) {
				it = context[context.size()-this->experiment_hook_state_scope_contexts[n_index].size()].experiment_state_vals
					.insert({this->experiment_hook_state_indexes[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_hook_state_defs[n_index]->networks[this->experiment_hook_state_network_indexes[n_index]];
			state_network->activate(history->obs_snapshot,
									it->second);
		}
	}

	curr_node_id = this->next_node_id;

	if (this->experiment != NULL && run_helper.phase != RUN_PHASE_UPDATE) {
		context.back().node_id = -1;

		BranchExperimentHistory* branch_experiment_history = NULL;
		this->experiment->activate(curr_node_id,
								   problem,
								   context,
								   exit_depth,
								   exit_node_id,
								   run_helper,
								   branch_experiment_history);
		history->branch_experiment_history = branch_experiment_history;
	}
}
