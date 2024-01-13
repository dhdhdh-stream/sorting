#include "action_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "branch_experiment.h"
#include "clean_experiment.h"
#include "constants.h"
#include "state_network.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  ActionNodeHistory* history) {
	problem->perform_action(this->action);
	if (this->action.move != ACTION_NOOP) {
		run_helper.num_actions++;
	}
	history->obs_snapshot = problem->get_observation();

	history->state_snapshots = vector<double>(this->state_is_local.size(), 0.0);
	vector<map<Scope*, set<int>>> obs_impacted_potential_scopes(this->state_is_local.size());
	for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
		if (this->state_is_local[n_index]) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->state_indexes[n_index]);
			if (it == context.back().local_state_vals.end()) {
				it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;

				if (this->is_potential) {
					it->second.impacted_potential_scopes[this->parent] = set<int>({this->state_indexes[n_index]});
				}
			}
			StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
			if (this->state_obs_indexes[n_index] == -1) {
				state_network->activate(history->obs_snapshot,
										it->second);
			} else {
				state_network->activate(history->state_snapshots[this->state_obs_indexes[n_index]],
										it->second);

				it->second.update_impacted_potential_scopes(
					obs_impacted_potential_scopes[this->state_obs_indexes[n_index]]);
			}
			history->state_snapshots[n_index] = it->second.val;
			obs_impacted_potential_scopes[n_index] = it->second.impacted_potential_scopes;
		} else {
			map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->state_indexes[n_index]);
			if (it != context.back().input_state_vals.end()) {
				StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
				if (this->state_obs_indexes[n_index] == -1) {
					state_network->activate(history->obs_snapshot,
											it->second);
				} else {
					state_network->activate(history->state_snapshots[this->state_obs_indexes[n_index]],
											it->second);

					it->second.update_impacted_potential_scopes(
						obs_impacted_potential_scopes[this->state_obs_indexes[n_index]]);
				}
				history->state_snapshots[n_index] = it->second.val;
				obs_impacted_potential_scopes[n_index] = it->second.impacted_potential_scopes;
			}
		}
	}

	for (int n_index = 0; n_index < (int)this->temp_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->temp_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->temp_state_scope_contexts[n_index].size()-1; c_index++) {
				if (context[context.size()-this->temp_state_scope_contexts[n_index].size()+c_index].scope == NULL			// OuterExperiment edge case
						|| this->temp_state_scope_contexts[n_index][c_index] != context[context.size()-this->temp_state_scope_contexts[n_index].size()+c_index].scope->id
						|| context[context.size()-this->temp_state_scope_contexts[n_index].size()+c_index].node == NULL		// explore edge case
						|| this->temp_state_node_contexts[n_index][c_index] != context[context.size()-this->temp_state_scope_contexts[n_index].size()+c_index].node->id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<State*, StateStatus>::iterator it = context[context.size()-this->temp_state_scope_contexts[n_index].size()]
				.temp_state_vals.find(this->temp_state_defs[n_index]);
			if (it == context[context.size()-this->temp_state_scope_contexts[n_index].size()].temp_state_vals.end()) {
				it = context[context.size()-this->temp_state_scope_contexts[n_index].size()].temp_state_vals
					.insert({this->temp_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->temp_state_defs[n_index]->networks[this->temp_state_network_indexes[n_index]];
			if (this->temp_state_obs_indexes[n_index] == -1) {
				state_network->activate(history->obs_snapshot,
										it->second);
			} else {
				state_network->activate(history->state_snapshots[this->temp_state_obs_indexes[n_index]],
										it->second);

				it->second.update_impacted_potential_scopes(
					obs_impacted_potential_scopes[this->temp_state_obs_indexes[n_index]]);
			}
		}
	}

	for (int n_index = 0; n_index < (int)this->experiment_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_state_scope_contexts[n_index].size()-1; c_index++) {
				if (context[context.size()-this->experiment_state_scope_contexts[n_index].size()+c_index].scope == NULL
						|| this->experiment_state_scope_contexts[n_index][c_index] != context[context.size()-this->experiment_state_scope_contexts[n_index].size()+c_index].scope->id
						|| context[context.size()-this->experiment_state_scope_contexts[n_index].size()+c_index].node == NULL
						|| this->experiment_state_node_contexts[n_index][c_index] != context[context.size()-this->experiment_state_scope_contexts[n_index].size()+c_index].node->id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<State*, StateStatus>::iterator it = context[context.size()-this->experiment_state_scope_contexts[n_index].size()]
				.temp_state_vals.find(this->experiment_state_defs[n_index]);
			if (it == context[context.size()-this->experiment_state_scope_contexts[n_index].size()].temp_state_vals.end()) {
				it = context[context.size()-this->experiment_state_scope_contexts[n_index].size()].temp_state_vals
					.insert({this->experiment_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_state_defs[n_index]->networks[this->experiment_state_network_indexes[n_index]];
			if (this->experiment_state_obs_indexes[n_index] == -1) {
				state_network->activate(history->obs_snapshot,
										it->second);
			} else {
				state_network->activate(history->state_snapshots[this->experiment_state_obs_indexes[n_index]],
										it->second);

				it->second.update_impacted_potential_scopes(
					obs_impacted_potential_scopes[this->experiment_state_obs_indexes[n_index]]);
			}
		}
	}

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		if (this->experiment->type == EXPERIMENT_TYPE_BRANCH) {
			BranchExperiment* branch_experiment = (BranchExperiment*)this->experiment;
			branch_experiment->activate(curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper,
										history->experiment_history);
		} else if (this->experiment->type == EXPERIMENT_TYPE_PASS_THROUGH) {
			PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->experiment;
			pass_through_experiment->activate(curr_node,
											  problem,
											  context,
											  exit_depth,
											  exit_node,
											  run_helper,
											  history->experiment_history);
		} else {
			CleanExperiment* clean_experiment = (CleanExperiment*)this->experiment;
			clean_experiment->activate(curr_node,
									   context,
									   exit_depth,
									   exit_node,
									   run_helper);
		}
	}
}
