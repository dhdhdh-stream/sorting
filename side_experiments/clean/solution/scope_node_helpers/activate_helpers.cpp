#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "branch_experiment.h"
#include "constants.h"
#include "state_network.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void ScopeNode::activate(AbstractNode*& curr_node,
						 Problem* problem,
						 vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	map<int, StateStatus> input_state_vals;
	map<int, StateStatus> local_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;

					if (this->is_potential) {
						it->second.impacted_potential_scopes[this->parent] = set<int>{this->input_outer_indexes[i_index]};
					}
				}
				if (this->input_inner_is_local[i_index]) {
					local_state_vals[this->input_inner_indexes[i_index]] = state_status;
				} else {
					input_state_vals[this->input_inner_indexes[i_index]] = state_status;
				}
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					if (this->input_inner_is_local[i_index]) {
						local_state_vals[this->input_inner_indexes[i_index]] = it->second;
					} else {
						input_state_vals[this->input_inner_indexes[i_index]] = it->second;
					}
				}
			}
		} else {
			if (this->input_inner_is_local[i_index]) {
				local_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
			} else {
				input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index]);
			}
		}
	}

	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->inner_scope;
	context.back().node = NULL;

	context.back().input_state_vals = input_state_vals;
	context.back().local_state_vals = local_state_vals;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	this->inner_scope->activate(problem,
								context,
								inner_exit_depth,
								inner_exit_node,
								run_helper,
								inner_scope_history);

	for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
		if (this->output_inner_is_local[o_index]) {
			map<int, StateStatus>::iterator inner_it = context.back().local_state_vals.find(this->output_inner_indexes[o_index]);
			if (inner_it != context.back().local_state_vals.end()) {
				if (this->output_outer_is_local[o_index]) {
					context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;

					// if this->is_potential, impacted_potential_scopes must already be set from input
					// HERE
				} else {
					map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
					if (outer_it != context[context.size()-2].input_state_vals.end()) {
						outer_it->second = inner_it->second;
					}
				}
			}
		} else {
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
	}
	/**
	 * - intuitively, pass by reference out
	 *   - so keep even if early exit
	 * 
	 * - also will be how inner branches affect outer scopes on early exit
	 */

	for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
			it != context.back().input_state_vals.end(); it++) {
		if (it->second.used) {
			for (map<Scope*, set<int>>::iterator scope_it = it->second.impacted_potential_scopes.begin();
					scope_it != it->second.impacted_potential_scopes.end(); scope_it++) {
				for (set<int>::iterator index_it = scope_it->second.begin();
						index_it != scope_it->second.end(); index_it++) {
					scope_it->first->used_states[*index_it] = true;
				}
			}
		}
	}
	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		if (it->second.used) {
			for (map<Scope*, set<int>>::iterator scope_it = it->second.impacted_potential_scopes.begin();
					scope_it != it->second.impacted_potential_scopes.end(); scope_it++) {
				for (set<int>::iterator index_it = scope_it->second.begin();
						index_it != scope_it->second.end(); index_it++) {
					scope_it->first->used_states[*index_it] = true;
				}
			}
		}
	}
	for (map<State*, StateStatus>::iterator it = context.back().temp_state_vals.begin();
			it != context.back().temp_state_vals.end(); it++) {
		if (it->second.used) {
			for (map<Scope*, set<int>>::iterator scope_it = it->second.impacted_potential_scopes.begin();
					scope_it != it->second.impacted_potential_scopes.end(); scope_it++) {
				for (set<int>::iterator index_it = scope_it->second.begin();
						index_it != scope_it->second.end(); index_it++) {
					scope_it->first->used_states[*index_it] = true;
				}
			}
		}
	}

	context.pop_back();

	context.back().node = NULL;

	if (inner_exit_depth == -1 && !run_helper.exceeded_limit) {
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
			} else {
				// this->experiment->type == EXPERIMENT_TYPE_PASS_THROUGH
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->experiment;
				pass_through_experiment->activate(curr_node,
												  problem,
												  context,
												  exit_depth,
												  exit_node,
												  run_helper,
												  history->experiment_history);
			}
		}
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
