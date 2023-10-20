#include "branch_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "scale.h"
#include "scope.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void BranchNode::activate(int& curr_node_id,
						  Problem& problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  int& exit_node_id,
						  RunHelper& run_helper,
						  vector<AbstractNodeHistory*>& node_histories) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (this->branch_scope_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].scope_id
					|| this->branch_node_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].node_id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->recursion_protection
				&& run_helper.recursion_protection_flags.find(this) != run_helper.recursion_protection_flags.end()) {
			// is_branch == false
			curr_node_id = this->original_next_node_id;
		} else {
			if (this->branch_is_pass_through) {
				// is_branch == true
				curr_node_id = this->branch_next_node_id;

				if (this->recursion_protection) {
					context.back().added_recursion_protection_flags.push_back(this);
					run_helper.recursion_protection_flags.insert(this);
				}
			} else {
				BranchNodeHistory* history = new BranchNodeHistory(this);
				node_histories.push_back(history);

				double original_score = this->original_score_mod;
				double branch_score = this->branch_score_mod;

				for (int s_index = 0; s_index < (int)this->decision_state_is_local.size(); s_index++) {
					if (this->decision_state_is_local[s_index]) {
						map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->decision_state_indexes[s_index]);
						if (it != context.back().local_state_vals.end()) {
							StateNetwork* last_network = it->second.last_network;
							if (last_network != NULL) {
								double normalized = (it->second.val - last_network->ending_mean)
									/ last_network->ending_standard_deviation;
								original_score += this->decision_original_weights[s_index] * normalized;
								branch_score += this->decision_branch_weights[s_index] * normalized;
							} else {
								original_score += this->decision_original_weights[s_index] * it->second.val;
								branch_score += this->decision_branch_weights[s_index] * it->second.val;
							}
						}
					} else {
						map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->decision_state_indexes[s_index]);
						if (it != context.back().input_state_vals.end()) {
							StateNetwork* last_network = it->second.last_network;
							if (last_network != NULL) {
								double normalized = (it->second.val - last_network->ending_mean)
									/ last_network->ending_standard_deviation;
								original_score += this->decision_original_weights[s_index] * normalized;
								branch_score += this->decision_branch_weights[s_index] * normalized;
							} else {
								original_score += this->decision_original_weights[s_index] * it->second.val;
								branch_score += this->decision_branch_weights[s_index] * it->second.val;
							}
						}
					}
				}

				bool is_branch;
				if (branch_score > original_score) {
					is_branch = true;

					if (this->recursion_protection) {
						context.back().added_recursion_protection_flags.push_back(this);
						run_helper.recursion_protection_flags.insert(this);
					}
				} else {
					is_branch = false;
				}

				if (is_branch) {
					history->obs_snapshot = -1.0;
				} else {
					history->obs_snapshot = 1.0;
				}

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

				if (is_branch) {
					curr_node_id = this->branch_next_node_id;

					if (this->experiment != NULL
							&& run_helper.phase != RUN_PHASE_UPDATE
							&& this->experiment_is_branch) {
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
				} else {
					curr_node_id = this->original_next_node_id;

					if (this->experiment != NULL
							&& run_helper.phase != RUN_PHASE_UPDATE
							&& !this->experiment_is_branch) {
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
			}
		}
	} else {
		// is_branch == false
		curr_node_id = this->original_next_node_id;
	}
}
