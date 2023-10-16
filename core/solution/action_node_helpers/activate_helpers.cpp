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

		for (int n_index = 0; n_index < (int)this->score_state_defs.size(); n_index++) {
			bool matches_context = true;
			if (this->score_state_scope_contexts[n_index].size() > context.size()) {
				matches_context = false;
			} else {
				for (int c_index = 0; c_index < (int)this->score_state_scope_contexts[n_index].size()-1; c_index++) {
					if (this->score_state_scope_contexts[n_index][c_index] != context[context.size()-this->score_state_scope_contexts[n_index].size()+c_index].scope_id
							|| this->score_state_node_contexts[n_index][c_index] != context[context.size()-this->score_state_scope_contexts[n_index].size()+c_index].node_id) {
						matches_context = false;
						break;
					}
				}
			}

			if (matches_context) {
				map<State*, StateStatus>::iterator it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.find(this->score_state_defs[n_index]);
				if (it == context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.end()) {
					it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.insert({this->score_state_defs[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->score_state_defs[n_index]->networks[this->score_state_network_indexes[n_index]];
				StateStatus score_state_impact;
				state_network->activate(history->obs_snapshot,
										it->second,
										score_state_impact);
				history->score_state_indexes.push_back(n_index);
				history->score_state_impacts.push_back(score_state_impact);
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

		for (int n_index = 0; n_index < (int)this->score_state_defs.size(); n_index++) {
			bool matches_context = true;
			if (this->score_state_scope_contexts[n_index].size() > context.size()) {
				matches_context = false;
			} else {
				for (int c_index = 0; c_index < (int)this->score_state_scope_contexts[n_index].size()-1; c_index++) {
					if (this->score_state_scope_contexts[n_index][c_index] != context[context.size()-this->score_state_scope_contexts[n_index].size()+c_index].scope_id
							|| this->score_state_node_contexts[n_index][c_index] != context[context.size()-this->score_state_scope_contexts[n_index].size()+c_index].node_id) {
						matches_context = false;
						break;
					}
				}
			}

			if (matches_context) {
				map<State*, StateStatus>::iterator it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.find(this->score_state_defs[n_index]);
				if (it == context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.end()) {
					it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.insert({this->score_state_defs[n_index], StateStatus()}).first;
				}
				StateNetwork* state_network = this->score_state_defs[n_index]->networks[this->score_state_network_indexes[n_index]];
				state_network->activate(history->obs_snapshot,
										it->second);
			}
		}
	}
 
	for (int n_index = 0; n_index < (int)this->experiment_hook_score_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_hook_score_state_scope_contexts[n_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_hook_score_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_hook_score_state_scope_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index].scope_id
						|| this->experiment_hook_score_state_node_contexts[n_index][c_index] != context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<State*, StateStatus>::iterator it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()]
				.experiment_score_state_vals.find(this->experiment_hook_score_state_defs[n_index]);
			if (it == context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals.end()) {
				it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals
					.insert({this->experiment_hook_score_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_hook_score_state_defs[n_index]->networks[this->experiment_hook_score_state_network_indexes[n_index]];
			state_network->activate(history->obs_snapshot,
									it->second);
		}
	}

	for (int h_index = 0; h_index < (int)this->test_hook_indexes.size(); h_index++) {
		bool matches_context = true;
		if (this->test_hook_scope_contexts[h_index].size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->test_hook_scope_contexts[h_index].size()-1; c_index++) {
				if (this->test_hook_scope_contexts[h_index][c_index] != context[context.size()-this->test_hook_scope_contexts[h_index].size()+c_index].scope_id
						|| this->test_hook_node_contexts[h_index][c_index] != context[context.size()-this->test_hook_scope_contexts[h_index].size()+c_index].node_id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			context[context.size()-this->test_hook_scope_contexts[h_index].size()]
				.scope_history->test_obs_indexes.push_back(this->test_hook_indexes[h_index]);
			context[context.size()-this->test_hook_scope_contexts[h_index].size()]
				.scope_history->test_obs_vals.push_back(history->obs_snapshot);
		}
	}

	curr_node_id = this->next_node_id;

	if (this->experiment != NULL) {
		run_helper.node_index++;

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

void ActionNode::experiment_back_activate(vector<int>& scope_context,
										  vector<int>& node_context,
										  map<State*, StateStatus>& experiment_score_state_vals,
										  vector<int>& test_obs_indexes,
										  vector<double>& test_obs_vals,
										  RunHelper& run_helper,
										  ActionNodeHistory* history) {
	for (int n_index = 0; n_index < (int)this->experiment_hook_score_state_defs.size(); n_index++) {
		bool matches_context = true;
		if (this->experiment_hook_score_state_scope_contexts[n_index].size() > scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->experiment_hook_score_state_scope_contexts[n_index].size()-1; c_index++) {
				if (this->experiment_hook_score_state_scope_contexts[n_index][c_index] != scope_context[scope_context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index]
						|| this->experiment_hook_score_state_node_contexts[n_index][c_index] != node_context[scope_context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			map<State*, StateStatus>::iterator it = experiment_score_state_vals.find(this->experiment_hook_score_state_defs[n_index]);
			if (it == experiment_score_state_vals.end()) {
				it = experiment_score_state_vals.insert({this->experiment_hook_score_state_defs[n_index], StateStatus()}).first;
			}
			StateNetwork* state_network = this->experiment_hook_score_state_defs[n_index]->networks[this->experiment_hook_score_state_network_indexes[n_index]];
			state_network->activate(history->obs_snapshot,
									it->second);
			/**
			 * - simply set to node index right before experiment
			 */
		}
	}

	for (int h_index = 0; h_index < (int)this->test_hook_indexes.size(); h_index++) {
		bool matches_context = true;
		if (this->test_hook_scope_contexts[h_index].size() > scope_context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->test_hook_scope_contexts[h_index].size()-1; c_index++) {
				if (this->test_hook_scope_contexts[h_index][c_index] != scope_context[scope_context.size()-this->test_hook_scope_contexts[h_index].size()+c_index]
						|| this->test_hook_node_contexts[h_index][c_index] != node_context[scope_context.size()-this->test_hook_scope_contexts[h_index].size()+c_index]) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			test_obs_indexes.push_back(this->test_hook_indexes[h_index]);
			test_obs_vals.push_back(history->obs_snapshot);
		}
	}
}
