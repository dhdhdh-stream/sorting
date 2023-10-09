#include "scope_node.h"

#include <iostream>

#include "branch_experiment.h"
#include "scope.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void ScopeNode::activate(int& curr_node_id,
						 Problem& problem,
						 vector<ContextLayer>& context,
						 int& exit_depth,
						 int& exit_node_id,
						 RunHelper& run_helper,
						 vector<AbstractNodeHistory*>& node_histories) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	node_histories.push_back(history);

	history->is_halfway = false;

	vector<map<int, StateStatus>> inner_input_state_vals(this->starting_node_ids.size());
	vector<map<int, StateStatus>> inner_local_state_vals(this->starting_node_ids.size());
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				if (this->input_inner_is_local[i_index]) {
					inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = state_status;
				} else {
					inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = state_status;
				}
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					if (this->input_inner_is_local[i_index]) {
						inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = it->second;
					} else {
						inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = it->second;
					}
				}
			}
		} else {
			if (this->input_inner_is_local[i_index]) {
				inner_local_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
			} else {
				inner_input_state_vals[this->input_inner_layers[i_index]][this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
			}
		}
	}

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope->id;
	context.back().node_id = -1;

	context.back().input_state_vals = inner_input_state_vals[0];
	inner_input_state_vals.erase(inner_input_state_vals.begin());
	context.back().local_state_vals = inner_local_state_vals[0];
	inner_local_state_vals.erase(inner_local_state_vals.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(this->inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	vector<int> starting_node_ids_copy = this->starting_node_ids;

	// currently, starting_node_ids.size() == inner_state_vals.size()+1

	int inner_exit_depth = -1;
	int inner_exit_node_id = -1;

	this->inner_scope->activate(starting_node_ids_copy,
								inner_input_state_vals,
								inner_local_state_vals,
								problem,
								context,
								inner_exit_depth,
								inner_exit_node_id,
								run_helper,
								inner_scope_history);

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
			} else {
				map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2].input_state_vals.end()) {
					outer_it->second = inner_it->second;
				}
			}
		}
	}
	/**
	 * - intuitively, pass by reference out
	 *   - so keep even if early exit
	 * 
	 * - also will be how inner branches affect outer scopes on early exit
	 */

	if (inner_exit_depth == -1) {
		history->is_early_exit = false;
	} else {
		history->is_early_exit = true;
	}

	if (!history->is_early_exit) {
		history->obs_snapshots = context.back().local_state_vals;
	}

	context.pop_back();

	if (!history->is_early_exit) {
		for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
			map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->state_obs_indexes[n_index]);
			if (obs_it != history->obs_snapshots.end()) {
				if (this->state_is_local[n_index]) {
					map<int, StateStatus>::iterator state_it = context.back().local_state_vals.find(this->state_indexes[n_index]);
					if (state_it == context.back().local_state_vals.end()) {
						state_it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
					}
					StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
					state_network->activate(obs_it->second.val,
											state_it->second);
				} else {
					map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_indexes[n_index]);
					if (state_it != context.back().input_state_vals.end()) {
						StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
						state_network->activate(obs_it->second.val,
												state_it->second);
					}
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
				map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->score_state_obs_indexes[n_index]);
				if (obs_it != history->obs_snapshots.end()) {
					map<State*, StateStatus>::iterator state_it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.find(this->score_state_defs[n_index]);
					if (state_it == context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.end()) {
						state_it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.insert({this->score_state_defs[n_index], StateStatus()}).first;
					}
					StateNetwork* state_network = this->score_state_defs[n_index]->networks[this->score_state_network_indexes[n_index]];
					StateStatus score_state_impact;
					state_network->activate_score(obs_it->second.val,
												  state_it->second,
												  score_state_impact);
					state_it->second.last_updated = run_helper.node_index;
					history->score_state_indexes.push_back(n_index);
					history->score_state_impacts.push_back(score_state_impact);
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
				map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->experiment_hook_score_state_obs_indexes[n_index]);
				if (obs_it != history->obs_snapshots.end()) {
					map<State*, StateStatus>::iterator state_it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()]
						.experiment_score_state_vals.find(this->experiment_hook_score_state_defs[n_index]);
					if (state_it == context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals.end()) {
						state_it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals
							.insert({this->experiment_hook_score_state_defs[n_index], StateStatus()}).first;
					}
					StateNetwork* state_network = this->experiment_hook_score_state_defs[n_index]->networks[this->experiment_hook_score_state_network_indexes[n_index]];
					state_network->activate(obs_it->second.val,
											state_it->second);
					state_it->second.last_updated = run_helper.node_index;
				}
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
				map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->test_hook_obs_indexes[h_index]);
				if (obs_it != history->obs_snapshots.end()) {
					context[context.size()-this->test_hook_scope_contexts[h_index].size()]
						.scope_history->test_obs_indexes.push_back(this->test_hook_indexes[h_index]);
					context[context.size()-this->test_hook_scope_contexts[h_index].size()]
						.scope_history->test_obs_vals.push_back(obs_it->second.val);
					context[context.size()-this->test_hook_scope_contexts[h_index].size()]
						.scope_history->test_last_updated = run_helper.node_index;
				}
			}
		}

		curr_node_id = this->next_node_id;

		if (this->experiment != NULL) {
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
	} else if (inner_exit_depth == 0) {
		curr_node_id = inner_exit_node_id;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node_id = inner_exit_node_id;
	}
}

void ScopeNode::halfway_activate(vector<int>& starting_node_ids,
								 vector<map<int, StateStatus>>& starting_input_state_vals,
								 vector<map<int, StateStatus>>& starting_local_state_vals,
								 int& curr_node_id,
								 Problem& problem,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 int& exit_node_id,
								 RunHelper& run_helper,
								 vector<AbstractNodeHistory*>& node_histories) {
	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	node_histories.push_back(history);

	history->is_halfway = true;

	context.push_back(ContextLayer());

	context.back().scope_id = this->inner_scope->id;
	context.back().node_id = -1;

	context.back().input_state_vals = starting_input_state_vals[0];
	starting_input_state_vals.erase(starting_input_state_vals.begin());
	context.back().local_state_vals = starting_local_state_vals[0];
	starting_local_state_vals.erase(starting_local_state_vals.begin());

	ScopeHistory* inner_scope_history = new ScopeHistory(this->inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	// currently, starting_node_ids.size() == starting_state_vals.size()+1

	int inner_exit_depth = -1;
	int inner_exit_node_id = -1;

	this->inner_scope->activate(starting_node_ids,
								starting_input_state_vals,
								starting_local_state_vals,
								problem,
								context,
								inner_exit_depth,
								inner_exit_node_id,
								run_helper,
								inner_scope_history);

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
		if (inner_it != context.back().input_state_vals.end()) {
			if (this->output_outer_is_local[o_index]) {
				context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
			} else {
				map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
				if (outer_it != context[context.size()-2].input_state_vals.end()) {
					outer_it->second = inner_it->second;
				}
			}
		}
	}

	if (inner_exit_depth == -1) {
		history->is_early_exit = false;
	} else {
		history->is_early_exit = true;
	}

	if (!history->is_early_exit) {
		history->obs_snapshots = context.back().local_state_vals;
	}

	context.pop_back();

	if (!history->is_early_exit) {
		for (int n_index = 0; n_index < (int)this->state_is_local.size(); n_index++) {
			map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->state_obs_indexes[n_index]);
			if (obs_it != history->obs_snapshots.end()) {
				if (this->state_is_local[n_index]) {
					map<int, StateStatus>::iterator state_it = context.back().local_state_vals.find(this->state_indexes[n_index]);
					if (state_it == context.back().local_state_vals.end()) {
						state_it = context.back().local_state_vals.insert({this->state_indexes[n_index], StateStatus()}).first;
					}
					StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
					state_network->activate(obs_it->second.val,
											state_it->second);
				} else {
					map<int, StateStatus>::iterator state_it = context.back().input_state_vals.find(this->state_indexes[n_index]);
					if (state_it != context.back().input_state_vals.end()) {
						StateNetwork* state_network = this->state_defs[n_index]->networks[this->state_network_indexes[n_index]];
						state_network->activate(obs_it->second.val,
												state_it->second);
					}
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
				map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->score_state_obs_indexes[n_index]);
				if (obs_it != history->obs_snapshots.end()) {
					map<State*, StateStatus>::iterator state_it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.find(this->score_state_defs[n_index]);
					if (state_it == context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.end()) {
						state_it = context[context.size()-this->score_state_scope_contexts[n_index].size()].score_state_vals.insert({this->score_state_defs[n_index], StateStatus()}).first;
					}
					StateNetwork* state_network = this->score_state_defs[n_index]->networks[this->score_state_network_indexes[n_index]];
					StateStatus score_state_impact;
					state_network->activate_score(obs_it->second.val,
												  state_it->second,
												  score_state_impact);
					state_it->second.last_updated = run_helper.node_index;
					history->score_state_indexes.push_back(n_index);
					history->score_state_impacts.push_back(score_state_impact);
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
				map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->experiment_hook_score_state_obs_indexes[n_index]);
				if (obs_it != history->obs_snapshots.end()) {
					map<State*, StateStatus>::iterator state_it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()]
						.experiment_score_state_vals.find(this->experiment_hook_score_state_defs[n_index]);
					if (state_it == context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals.end()) {
						state_it = context[context.size()-this->experiment_hook_score_state_scope_contexts[n_index].size()].experiment_score_state_vals
							.insert({this->experiment_hook_score_state_defs[n_index], StateStatus()}).first;
					}
					StateNetwork* state_network = this->experiment_hook_score_state_defs[n_index]->networks[this->experiment_hook_score_state_network_indexes[n_index]];
					state_network->activate(obs_it->second.val,
											state_it->second);
					state_it->second.last_updated = run_helper.node_index;
				}
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
				map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->test_hook_obs_indexes[h_index]);
				if (obs_it != history->obs_snapshots.end()) {
					context[context.size()-this->test_hook_scope_contexts[h_index].size()]
						.scope_history->test_obs_indexes.push_back(this->test_hook_indexes[h_index]);
					context[context.size()-this->test_hook_scope_contexts[h_index].size()]
						.scope_history->test_obs_vals.push_back(obs_it->second.val);
					context[context.size()-this->test_hook_scope_contexts[h_index].size()]
						.scope_history->test_last_updated = run_helper.node_index;
				}
			}
		}

		curr_node_id = this->next_node_id;

		if (this->experiment != NULL) {
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
	} else if (inner_exit_depth == 0) {
		curr_node_id = inner_exit_node_id;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node_id = inner_exit_node_id;
	}
}

void ScopeNode::experiment_back_activate(vector<int>& scope_context,
										 vector<int>& node_context,
										 map<State*, StateStatus>& experiment_score_state_vals,
										 vector<int>& test_obs_indexes,
										 vector<double>& test_obs_vals,
										 int& test_last_updated,
										 RunHelper& run_helper,
										 ScopeNodeHistory* history) {
	if (!history->is_early_exit) {
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
				map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->experiment_hook_score_state_obs_indexes[n_index]);
				if (obs_it != history->obs_snapshots.end()) {
					map<State*, StateStatus>::iterator state_it = experiment_score_state_vals.find(this->experiment_hook_score_state_defs[n_index]);
					if (state_it == experiment_score_state_vals.end()) {
						state_it = experiment_score_state_vals.insert({this->experiment_hook_score_state_defs[n_index], StateStatus()}).first;
					}
					StateNetwork* state_network = this->experiment_hook_score_state_defs[n_index]->networks[this->experiment_hook_score_state_network_indexes[n_index]];
					state_network->activate(obs_it->second.val,
											state_it->second);
					state_it->second.last_updated = run_helper.node_index;
				}
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
				map<int, StateStatus>::iterator obs_it = history->obs_snapshots.find(this->test_hook_obs_indexes[h_index]);
				if (obs_it != history->obs_snapshots.end()) {
					test_obs_indexes.push_back(this->test_hook_indexes[h_index]);
					test_obs_vals.push_back(obs_it->second.val);
					test_last_updated = run_helper.node_index;
				}
			}
		}
	}
}
