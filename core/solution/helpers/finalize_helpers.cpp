/**
 * - at containing scope, new inner state corresponds 1-to-1 with outer across new sequences/branch
 */

#include "helpers.h"

#include <iostream>
#include <set>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

void add_state_local_scope_node_helper(ScopeNode* scope_node,
									   int local_index,
									   int inner_index,
									   set<int>& updated_local_scope_nodes) {
	set<int>::iterator it = updated_local_scope_nodes.find(scope_node->id);
	if (it == updated_local_scope_nodes.end()) {
		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_indexes.push_back(inner_index);
		scope_node->input_outer_is_local.push_back(true);
		scope_node->input_outer_indexes.push_back(local_index);
		scope_node->input_init_vals.push_back(0.0);
		scope_node->input_init_index_vals.push_back(0.0);

		scope_node->output_inner_indexes.push_back(inner_index);
		scope_node->output_outer_is_local.push_back(true);
		scope_node->output_outer_indexes.push_back(local_index);

		updated_local_scope_nodes.insert(scope_node->id);
	}
}

void add_state_input_scope_node_helper(int c_index,
									   vector<int>& scope_context,
									   vector<int>& node_context,
									   int outer_index,
									   int inner_index,
									   set<pair<vector<int>,vector<int>>>& updated_input_scope_nodes) {
	vector<int> curr_scope_context(scope_context.begin(), scope_context.begin()+c_index+1);
	vector<int> curr_node_context(node_context.begin(), node_context.begin()+c_index+1);
	set<pair<vector<int>,vector<int>>>::iterator it = updated_input_scope_nodes.find({curr_scope_context, curr_node_context});
	if (it == updated_input_scope_nodes.end()) {
		Scope* scope = solution->scopes[scope_context[c_index]];
		ScopeNode* scope_node = (ScopeNode*)scope->nodes[node_context[c_index]];

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_indexes.push_back(inner_index);
		scope_node->input_outer_is_local.push_back(false);
		scope_node->input_outer_indexes.push_back(outer_index);
		scope_node->input_init_vals.push_back(0.0);
		scope_node->input_init_index_vals.push_back(0.0);

		scope_node->output_inner_indexes.push_back(inner_index);
		scope_node->output_outer_is_local.push_back(false);
		scope_node->output_outer_indexes.push_back(outer_index);

		updated_input_scope_nodes.insert({curr_scope_context, curr_node_context});
	}
}

int add_state_scope_helper(int c_index,
						   vector<int>& scope_context,
						   vector<int>& node_context,
						   map<pair<vector<int>,vector<int>>, int>& updated_scopes) {
	vector<int> curr_scope_context(scope_context.begin(), scope_context.begin()+c_index+1);
	/**
	 * - don't include last layer for node context
	 */
	vector<int> curr_node_context(node_context.begin(), node_context.begin()+c_index);
	map<pair<vector<int>,vector<int>>, int>::iterator it = updated_scopes.find({curr_scope_context, curr_node_context});
	if (it != updated_scopes.end()) {
		return it->second;
	} else {
		Scope* scope = solution->scopes[scope_context[c_index]];

		int new_input_index = scope->num_input_states;
		scope->num_input_states++;

		updated_scopes[{curr_scope_context, curr_node_context}] = new_input_index;
		return new_input_index;
	}
}

void add_state(Scope* parent_scope,
			   int temp_state_index,
			   vector<int>& experiment_scope_context,
			   vector<int>& experiment_node_context,
			   int outer_scope_depth,
			   int& new_state_index) {
	State* new_state = parent_scope->temp_states[temp_state_index];

	solution->states[new_state->id] = new_state;

	int new_local_index = parent_scope->num_local_states;
	parent_scope->num_local_states++;
	parent_scope->temp_state_new_local_indexes[temp_state_index] = new_local_index;

	set<int> updated_local_scope_nodes;
	set<pair<vector<int>,vector<int>>> updated_input_scope_nodes;
	map<pair<vector<int>,vector<int>>, int> updated_scopes;
	updated_scopes[{vector<int>{parent_scope->id}, vector<int>{}}] = new_local_index;

	vector<ActionNode*> nodes = parent_scope->temp_state_nodes[temp_state_index];
	vector<vector<int>> scope_contexts = parent_scope->temp_state_scope_contexts[temp_state_index];
	vector<vector<int>> node_contexts = parent_scope->temp_state_node_contexts[temp_state_index];
	vector<int> obs_indexes = parent_scope->temp_state_obs_indexes[temp_state_index];
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		if (scope_contexts[n_index].size() == 1) {
			nodes[n_index]->state_is_local.push_back(true);
			nodes[n_index]->state_indexes.push_back(new_local_index);
			nodes[n_index]->state_obs_indexes.push_back(obs_indexes[n_index]);
			nodes[n_index]->state_defs.push_back(new_state);
			nodes[n_index]->state_network_indexes.push_back(n_index);
		} else {
			{
				ScopeNode* local_scope_node = (ScopeNode*)parent_scope->nodes[node_contexts[n_index][0]];

				int inner_index = add_state_scope_helper(1,
														 scope_contexts[n_index],
														 node_contexts[n_index],
														 updated_scopes);

				add_state_local_scope_node_helper(local_scope_node,
												  new_local_index,
												  inner_index,
												  updated_local_scope_nodes);
			}
			for (int c_index = 1; c_index < (int)scope_contexts[n_index].size()-1; c_index++) {
				int outer_index = add_state_scope_helper(c_index,
														 scope_contexts[n_index],
														 node_contexts[n_index],
														 updated_scopes);

				int inner_index = add_state_scope_helper(c_index+1,
														 scope_contexts[n_index],
														 node_contexts[n_index],
														 updated_scopes);

				add_state_input_scope_node_helper(c_index,
												  scope_contexts[n_index],
												  node_contexts[n_index],
												  outer_index,
												  inner_index,
												  updated_input_scope_nodes);
			}
			{
				int outer_index = add_state_scope_helper(scope_contexts[n_index].size()-1,
														 scope_contexts[n_index],
														 node_contexts[n_index],
														 updated_scopes);

				nodes[n_index]->state_is_local.push_back(false);
				nodes[n_index]->state_indexes.push_back(outer_index);
				nodes[n_index]->state_obs_indexes.push_back(obs_indexes[n_index]);
				nodes[n_index]->state_defs.push_back(new_state);
				nodes[n_index]->state_network_indexes.push_back(n_index);
			}
		}
	}

	if (outer_scope_depth > 0) {
		int local_layer_index = experiment_scope_context.size()-1 - outer_scope_depth;
		vector<int> adjusted_scope_context(experiment_scope_context.begin()+local_layer_index, experiment_scope_context.end());
		vector<int> adjusted_node_context(experiment_node_context.begin()+local_layer_index, experiment_node_context.end());
		{
			ScopeNode* local_scope_node = (ScopeNode*)parent_scope->nodes[adjusted_node_context[0]];

			int inner_index = add_state_scope_helper(1,
													 adjusted_scope_context,
													 adjusted_node_context,
													 updated_scopes);

			add_state_local_scope_node_helper(local_scope_node,
											  new_local_index,
											  inner_index,
											  updated_local_scope_nodes);
		}
		for (int c_index = 1; c_index < (int)adjusted_scope_context.size()-1; c_index++) {
			int outer_index = add_state_scope_helper(c_index,
													 adjusted_scope_context,
													 adjusted_node_context,
													 updated_scopes);

			int inner_index = add_state_scope_helper(c_index+1,
													 adjusted_scope_context,
													 adjusted_node_context,
													 updated_scopes);

			add_state_input_scope_node_helper(c_index,
											  adjusted_scope_context,
											  adjusted_node_context,
											  outer_index,
											  inner_index,
											  updated_input_scope_nodes);
		}
		{
			int outer_index = add_state_scope_helper(adjusted_scope_context.size()-1,
													 adjusted_scope_context,
													 adjusted_node_context,
													 updated_scopes);

			new_state_index = outer_index;
		}
	}
	/**
	 * - for temp state, always at least added as input
	 */
}

void add_new_input(vector<int>& experiment_scope_context,
				   vector<int>& experiment_node_context,
				   int scope_depth,
				   int outer_is_local,
				   int outer_index,
				   int& new_state_index) {
	{
		int context_index = (int)experiment_scope_context.size()-1 - scope_depth;
		Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];
		ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[experiment_node_context[context_index]];
		Scope* inner_scope = scope_node->inner_scope;

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
		// not incremented yet
		scope_node->input_outer_is_local.push_back(outer_is_local);
		scope_node->input_outer_indexes.push_back(outer_index);
		scope_node->input_init_vals.push_back(0.0);
		scope_node->input_init_index_vals.push_back(0.0);
	}
	for (int c_index = scope_depth-1; c_index >= 1; c_index--) {
		int context_index = (int)experiment_scope_context.size()-1 - c_index;
		Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];
		ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[experiment_node_context[context_index]];
		Scope* inner_scope = scope_node->inner_scope;

		int new_input_index = outer_scope->num_input_states;
		outer_scope->num_input_states++;

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
		// not incremented yet
		scope_node->input_outer_is_local.push_back(false);
		scope_node->input_outer_indexes.push_back(new_input_index);
		scope_node->input_init_vals.push_back(0.0);
		scope_node->input_init_index_vals.push_back(0.0);
	}
	{
		Scope* outer_scope = solution->scopes[experiment_scope_context.back()];

		int new_input_index = outer_scope->num_input_states;
		outer_scope->num_input_states++;

		new_state_index = new_input_index;
	}
}

void add_existing_output(int new_state_index,
						 vector<int>& experiment_scope_context,
						 vector<int>& experiment_node_context,
						 int scope_depth,
						 int outer_is_local,
						 int outer_index) {
	int curr_inner_index;
	{
		curr_inner_index = new_state_index;
	}
	for (int c_index = 1; c_index < scope_depth-1; c_index++) {
		int context_index = (int)experiment_scope_context.size()-1 - c_index;
		Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];
		ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[experiment_node_context[context_index]];

		int curr_outer_index;
		for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
			if (scope_node->input_types[i_index] == INPUT_TYPE_STATE
					&& scope_node->input_inner_indexes[i_index] == curr_inner_index) {
				curr_outer_index = scope_node->input_outer_indexes[i_index];
				break;
			}
		}

		scope_node->output_inner_indexes.push_back(curr_inner_index);
		scope_node->output_outer_is_local.push_back(false);
		scope_node->output_outer_indexes.push_back(curr_outer_index);

		curr_inner_index = curr_outer_index;
	}
	{
		int context_index = (int)experiment_scope_context.size()-1 - scope_depth;
		Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];
		ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[experiment_node_context[context_index]];

		scope_node->output_inner_indexes.push_back(curr_inner_index);
		scope_node->output_outer_is_local.push_back(outer_is_local);
		scope_node->output_outer_indexes.push_back(outer_index);
	}
}

void add_new_output(vector<int>& experiment_scope_context,
					vector<int>& experiment_node_context,
					int scope_depth,
					int outer_is_local,
					int outer_index,
					int& new_state_index) {
	{
		int context_index = (int)experiment_scope_context.size()-1 - scope_depth;
		Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];
		ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[experiment_node_context[context_index]];
		Scope* inner_scope = scope_node->inner_scope;

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->input_outer_is_local.push_back(outer_is_local);
		scope_node->input_outer_indexes.push_back(outer_index);
		scope_node->input_init_vals.push_back(0.0);
		scope_node->input_init_index_vals.push_back(0.0);

		scope_node->output_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->output_outer_is_local.push_back(outer_is_local);
		scope_node->output_outer_indexes.push_back(outer_index);
	}
	for (int c_index = scope_depth-1; c_index >= 1; c_index--) {
		int context_index = (int)experiment_scope_context.size()-1 - c_index;
		Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];
		ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[experiment_node_context[context_index]];
		Scope* inner_scope = scope_node->inner_scope;

		int new_input_index = outer_scope->num_input_states;
		outer_scope->num_input_states++;

		scope_node->input_types.push_back(INPUT_TYPE_STATE);
		scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->input_outer_is_local.push_back(false);
		scope_node->input_outer_indexes.push_back(new_input_index);
		scope_node->input_init_vals.push_back(0.0);
		scope_node->input_init_index_vals.push_back(0.0);

		scope_node->output_inner_indexes.push_back(inner_scope->num_input_states);
		scope_node->output_outer_is_local.push_back(false);
		scope_node->output_outer_indexes.push_back(new_input_index);
	}
	{
		Scope* outer_scope = solution->scopes[experiment_scope_context.back()];

		int new_input_index = outer_scope->num_input_states;
		outer_scope->num_input_states++;

		new_state_index = new_input_index;
	}
}

void finalize_potential_scope(vector<int>& experiment_scope_context,
							  vector<int>& experiment_node_context,
							  PotentialScopeNode* potential_scope_node,
							  map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
							  map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	ScopeNode* new_scope_node = potential_scope_node->scope_node_placeholder;

	solution->scopes[potential_scope_node->scope->id] = potential_scope_node->scope;
	new_scope_node->inner_scope = potential_scope_node->scope;
	potential_scope_node->scope = NULL;

	for (int i_index = 0; i_index < (int)potential_scope_node->input_types.size(); i_index++) {
		if (potential_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
			bool outer_is_local;
			int outer_index;
			if (potential_scope_node->input_outer_types[i_index] == OUTER_TYPE_INPUT) {
				outer_is_local = false;
				outer_index = (long)potential_scope_node->input_outer_indexes[i_index];
			} else if (potential_scope_node->input_outer_types[i_index] == OUTER_TYPE_LOCAL) {
				outer_is_local = true;
				outer_index = (long)potential_scope_node->input_outer_indexes[i_index];
			} else {
				int context_index = (int)experiment_scope_context.size()-1 - potential_scope_node->input_scope_depths[i_index];
				Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];

				int temp_state_index;
				for (int t_index = 0; t_index < (int)outer_scope->temp_states.size(); t_index++) {
					if (outer_scope->temp_states[t_index] == potential_scope_node->input_outer_indexes[i_index]) {
						temp_state_index = t_index;
						break;
					}
				}

				if (outer_scope->temp_state_new_local_indexes[temp_state_index] == -1) {
					int new_state_index;
					add_state(outer_scope,
							  temp_state_index,
							  experiment_scope_context,
							  experiment_node_context,
							  potential_scope_node->input_scope_depths[i_index],
							  new_state_index);

					if (potential_scope_node->input_scope_depths[i_index] > 0) {
						input_scope_depths_mappings[{potential_scope_node->input_scope_depths[i_index],
							{true, outer_scope->temp_state_new_local_indexes[temp_state_index]}}] = new_state_index;
					}
				}

				outer_is_local = true;
				outer_index = outer_scope->temp_state_new_local_indexes[temp_state_index];
			}

			if (potential_scope_node->input_scope_depths[i_index] == 0) {
				new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_scope_node->input_inner_indexes.push_back(potential_scope_node->input_inner_indexes[i_index]);
				new_scope_node->input_outer_is_local.push_back(outer_is_local);
				new_scope_node->input_outer_indexes.push_back(outer_index);
				new_scope_node->input_init_vals.push_back(0.0);
				new_scope_node->input_init_index_vals.push_back(0.0);
			} else {
				int new_state_index;
				map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
					.find({potential_scope_node->input_scope_depths[i_index], {outer_is_local, outer_index}});
				if (input_it != input_scope_depths_mappings.end()) {
					new_state_index = input_it->second;
				} else {
					add_new_input(experiment_scope_context,
								  experiment_node_context,
								  potential_scope_node->input_scope_depths[i_index],
								  outer_is_local,
								  outer_index,
								  new_state_index);

					input_scope_depths_mappings[{potential_scope_node->input_scope_depths[i_index], {outer_is_local, outer_index}}] = new_state_index;
				}

				new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_scope_node->input_inner_indexes.push_back(potential_scope_node->input_inner_indexes[i_index]);
				new_scope_node->input_outer_is_local.push_back(false);
				new_scope_node->input_outer_indexes.push_back(new_state_index);
				new_scope_node->input_init_vals.push_back(0.0);
				new_scope_node->input_init_index_vals.push_back(0.0);
			}
		} else {
			new_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
			new_scope_node->input_inner_indexes.push_back(potential_scope_node->input_inner_indexes[i_index]);
			new_scope_node->input_outer_is_local.push_back(false);
			new_scope_node->input_outer_indexes.push_back(-1);
			new_scope_node->input_init_vals.push_back(potential_scope_node->input_init_vals[i_index]);
			new_scope_node->input_init_index_vals.push_back(potential_scope_node->input_init_index_vals[i_index]);
		}
	}

	for (int o_index = 0; o_index < (int)potential_scope_node->output_inner_indexes.size(); o_index++) {
		bool outer_is_local;
		int outer_index;
		if (potential_scope_node->output_outer_types[o_index] == OUTER_TYPE_INPUT) {
			outer_is_local = false;
			outer_index = (long)potential_scope_node->output_outer_indexes[o_index];
		} else if (potential_scope_node->output_outer_types[o_index] == OUTER_TYPE_LOCAL) {
			outer_is_local = true;
			outer_index = (long)potential_scope_node->output_outer_indexes[o_index];
		} else {
			int context_index = (int)experiment_scope_context.size()-1 - potential_scope_node->output_scope_depths[o_index];
			Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];

			int temp_state_index;
			for (int t_index = 0; t_index < (int)outer_scope->temp_states.size(); t_index++) {
				if (outer_scope->temp_states[t_index] == potential_scope_node->output_outer_indexes[o_index]) {
					temp_state_index = t_index;
					break;
				}
			}
			/**
			 * - temp_state must already been added
			 *   - i.e., outer_scope->temp_state_new_local_indexes[temp_state_index] != -1
			 */

			outer_is_local = true;
			outer_index = outer_scope->temp_state_new_local_indexes[temp_state_index];
		}

		if (potential_scope_node->output_scope_depths[o_index] == 0) {
			new_scope_node->output_inner_indexes.push_back(potential_scope_node->output_inner_indexes[o_index]);
			new_scope_node->output_outer_is_local.push_back(outer_is_local);
			new_scope_node->output_outer_indexes.push_back(outer_index);
		} else {
			int new_state_index;
			map<pair<int, pair<bool,int>>, int>::iterator output_it = output_scope_depths_mappings
				.find({potential_scope_node->output_scope_depths[o_index], {outer_is_local, outer_index}});
			if (output_it != output_scope_depths_mappings.end()) {
				new_state_index = output_it->second;
			} else {
				map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
					.find({potential_scope_node->output_scope_depths[o_index], {outer_is_local, outer_index}});
				if (input_it != input_scope_depths_mappings.end()) {
					output_scope_depths_mappings[{potential_scope_node->output_scope_depths[o_index], {outer_is_local, outer_index}}] = input_it->second;

					add_existing_output(input_it->second,
										experiment_scope_context,
										experiment_node_context,
										potential_scope_node->output_scope_depths[o_index],
										outer_is_local,
										outer_index);

					new_state_index = input_it->second;
				} else {
					add_new_output(experiment_scope_context,
								   experiment_node_context,
								   potential_scope_node->output_scope_depths[o_index],
								   outer_is_local,
								   outer_index,
								   new_state_index);

					input_scope_depths_mappings[{potential_scope_node->output_scope_depths[o_index], {outer_is_local, outer_index}}] = new_state_index;
					output_scope_depths_mappings[{potential_scope_node->output_scope_depths[o_index], {outer_is_local, outer_index}}] = new_state_index;
				}
			}

			new_scope_node->output_inner_indexes.push_back(potential_scope_node->output_inner_indexes[o_index]);
			new_scope_node->output_outer_is_local.push_back(false);
			new_scope_node->output_outer_indexes.push_back(new_state_index);
		}
	}
}

void duplicate_temp_state_helper(vector<int>& experiment_scope_context,
								 vector<int>& experiment_node_context,
								 int scope_depth,
								 int outer_index,
								 int& new_state_index) {
	int curr_index = -1;
	{
		int context_index = (int)experiment_scope_context.size()-1 - scope_depth;
		Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];
		ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[experiment_node_context[context_index]];

		int inner_index = -1;
		for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
			if (scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
				if (scope_node->input_outer_is_local[i_index] == true
						&& scope_node->input_outer_indexes[i_index] == outer_index) {
					inner_index = scope_node->input_inner_indexes[i_index];
					break;
				}
			}
		}

		if (inner_index == -1) {
			Scope* inner_scope = scope_node->inner_scope;

			scope_node->input_types.push_back(INPUT_TYPE_STATE);
			scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
			// not incremented yet
			scope_node->input_outer_is_local.push_back(true);
			scope_node->input_outer_indexes.push_back(outer_index);
			scope_node->input_init_vals.push_back(0.0);
			scope_node->input_init_index_vals.push_back(0.0);

			// leave curr_index as -1
		} else {
			curr_index = inner_index;
		}
	}
	for (int c_index = scope_depth-1; c_index >= 1; c_index--) {
		int context_index = (int)experiment_scope_context.size()-1 - c_index;
		Scope* outer_scope = solution->scopes[experiment_scope_context[context_index]];
		ScopeNode* scope_node = (ScopeNode*)outer_scope->nodes[experiment_node_context[context_index]];

		int inner_index = -1;
		if (curr_index != -1) {
			for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
				if (scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
					if (scope_node->input_outer_is_local[i_index] == false
							&& scope_node->input_outer_indexes[i_index] == curr_index) {
						inner_index = scope_node->input_inner_indexes[i_index];
						break;
					}
				}
			}
		}

		if (inner_index == -1) {
			Scope* inner_scope = scope_node->inner_scope;

			int new_input_index;
			if (curr_index == -1) {
				new_input_index = outer_scope->num_input_states;
				outer_scope->num_input_states++;
			} else {
				new_input_index = curr_index;
			}

			scope_node->input_types.push_back(INPUT_TYPE_STATE);
			scope_node->input_inner_indexes.push_back(inner_scope->num_input_states);
			// not incremented yet
			scope_node->input_outer_is_local.push_back(false);
			scope_node->input_outer_indexes.push_back(new_input_index);
			scope_node->input_init_vals.push_back(0.0);
			scope_node->input_init_index_vals.push_back(0.0);

			curr_index = -1;
		} else {
			curr_index = inner_index;
		}
	}
	{
		if (curr_index == -1) {
			Scope* outer_scope = solution->scopes[experiment_scope_context.back()];

			int new_input_index = outer_scope->num_input_states;
			outer_scope->num_input_states++;

			new_state_index = new_input_index;
		} else {
			new_state_index = curr_index;
		}
	}
}

void finalize_branch_node_states(BranchNode* new_branch_node,
								 vector<map<int, double>>& existing_input_state_weights,
								 vector<map<int, double>>& existing_local_state_weights,
								 vector<map<State*, double>>& existing_temp_state_weights,
								 vector<map<int, double>>& new_input_state_weights,
								 vector<map<int, double>>& new_local_state_weights,
								 vector<map<State*, double>>& new_temp_state_weights,
								 map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
								 map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	for (int c_index = 0; c_index < (int)new_branch_node->branch_scope_context.size()-1; c_index++) {
		int scope_depth = new_branch_node->branch_scope_context.size()-1 - c_index;
		for (map<int, double>::iterator existing_it = existing_input_state_weights[c_index].begin();
				existing_it != existing_input_state_weights[c_index].end(); existing_it++) {
			int new_state_index;
			map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
				.find({scope_depth, {false, existing_it->first}});
			if (input_it != input_scope_depths_mappings.end()) {
				new_state_index = input_it->second;
			} else {
				add_new_input(new_branch_node->branch_scope_context,
							  new_branch_node->branch_node_context,
							  scope_depth,
							  false,
							  existing_it->first,
							  new_state_index);

				input_scope_depths_mappings[{scope_depth, {false, existing_it->first}}] = new_state_index;
			}

			double existing_weight = existing_it->second;
			double new_weight = new_input_state_weights[c_index][existing_it->first];

			new_branch_node->decision_state_is_local.push_back(false);
			new_branch_node->decision_state_indexes.push_back(new_state_index);
			new_branch_node->decision_original_weights.push_back(existing_weight);
			new_branch_node->decision_branch_weights.push_back(new_weight);
		}
	}
	{
		for (map<int, double>::iterator existing_it = existing_input_state_weights.back().begin();
				existing_it != existing_input_state_weights.back().end(); existing_it++) {
			double existing_weight = existing_it->second;
			double new_weight = new_input_state_weights.back()[existing_it->first];

			new_branch_node->decision_state_is_local.push_back(false);
			new_branch_node->decision_state_indexes.push_back(existing_it->first);
			new_branch_node->decision_original_weights.push_back(existing_weight);
			new_branch_node->decision_branch_weights.push_back(new_weight);
		}
	}

	for (int c_index = 0; c_index < (int)new_branch_node->branch_scope_context.size()-1; c_index++) {
		int scope_depth = new_branch_node->branch_scope_context.size()-1 - c_index;
		for (map<int, double>::iterator existing_it = existing_local_state_weights[c_index].begin();
				existing_it != existing_local_state_weights[c_index].end(); existing_it++) {
			int new_state_index;
			map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
				.find({scope_depth, {true, existing_it->first}});
			if (input_it != input_scope_depths_mappings.end()) {
				new_state_index = input_it->second;
			} else {
				add_new_input(new_branch_node->branch_scope_context,
							  new_branch_node->branch_node_context,
							  scope_depth,
							  true,
							  existing_it->first,
							  new_state_index);

				input_scope_depths_mappings[{scope_depth, {true, existing_it->first}}] = new_state_index;
			}

			double existing_weight = existing_it->second;
			double new_weight = new_local_state_weights[c_index][existing_it->first];

			new_branch_node->decision_state_is_local.push_back(false);
			new_branch_node->decision_state_indexes.push_back(new_state_index);
			new_branch_node->decision_original_weights.push_back(existing_weight);
			new_branch_node->decision_branch_weights.push_back(new_weight);
		}
	}
	{
		for (map<int, double>::iterator existing_it = existing_local_state_weights.back().begin();
				existing_it != existing_local_state_weights.back().end(); existing_it++) {
			double existing_weight = existing_it->second;
			double new_weight = new_local_state_weights.back()[existing_it->first];

			new_branch_node->decision_state_is_local.push_back(true);
			new_branch_node->decision_state_indexes.push_back(existing_it->first);
			new_branch_node->decision_original_weights.push_back(existing_weight);
			new_branch_node->decision_branch_weights.push_back(new_weight);
		}
	}

	for (int c_index = 0; c_index < (int)new_branch_node->branch_scope_context.size()-1; c_index++) {
		int scope_depth = new_branch_node->branch_scope_context.size()-1 - c_index;
		for (map<State*, double>::iterator existing_it = existing_temp_state_weights[c_index].begin();
				existing_it != existing_temp_state_weights[c_index].end(); existing_it++) {
			Scope* outer_scope = solution->scopes[new_branch_node->branch_scope_context[c_index]];

			int temp_state_index;
			for (int t_index = 0; t_index < (int)outer_scope->temp_states.size(); t_index++) {
				if (outer_scope->temp_states[t_index] == existing_it->first) {
					temp_state_index = t_index;
					break;
				}
			}

			if (outer_scope->temp_state_new_local_indexes[temp_state_index] == -1) {
				int new_state_index;
				add_state(outer_scope,
						  temp_state_index,
						  new_branch_node->branch_scope_context,
						  new_branch_node->branch_node_context,
						  scope_depth,
						  new_state_index);

				if (scope_depth > 0) {
					input_scope_depths_mappings[{scope_depth,
						{true, outer_scope->temp_state_new_local_indexes[temp_state_index]}}] = new_state_index;
				}
			}

			int outer_index = outer_scope->temp_state_new_local_indexes[temp_state_index];

			int new_state_index;
			map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
				.find({scope_depth, {true, outer_index}});
			if (input_it != input_scope_depths_mappings.end()) {
				new_state_index = input_it->second;
			} else {
				duplicate_temp_state_helper(new_branch_node->branch_scope_context,
											new_branch_node->branch_node_context,
											scope_depth,
											outer_index,
											new_state_index);

				input_scope_depths_mappings[{scope_depth, {true, outer_index}}] = new_state_index;
			}

			double existing_weight = existing_it->second;
			double new_weight = new_temp_state_weights[c_index][existing_it->first];

			new_branch_node->decision_state_is_local.push_back(false);
			new_branch_node->decision_state_indexes.push_back(new_state_index);
			new_branch_node->decision_original_weights.push_back(existing_weight);
			new_branch_node->decision_branch_weights.push_back(new_weight);
		}
	}
	{
		for (map<State*, double>::iterator existing_it = existing_temp_state_weights.back().begin();
				existing_it != existing_temp_state_weights.back().end(); existing_it++) {
			Scope* outer_scope = solution->scopes[new_branch_node->branch_scope_context.back()];

			int temp_state_index;
			for (int t_index = 0; t_index < (int)outer_scope->temp_states.size(); t_index++) {
				if (outer_scope->temp_states[t_index] == existing_it->first) {
					temp_state_index = t_index;
					break;
				}
			}

			if (outer_scope->temp_state_new_local_indexes[temp_state_index] == -1) {
				int new_state_index;
				add_state(outer_scope,
						  temp_state_index,
						  new_branch_node->branch_scope_context,
						  new_branch_node->branch_node_context,
						  0,
						  new_state_index);
			}

			int outer_index = outer_scope->temp_state_new_local_indexes[temp_state_index];

			double existing_weight = existing_it->second;
			double new_weight = new_temp_state_weights.back()[existing_it->first];

			new_branch_node->decision_state_is_local.push_back(true);
			new_branch_node->decision_state_indexes.push_back(outer_index);
			new_branch_node->decision_original_weights.push_back(existing_weight);
			new_branch_node->decision_branch_weights.push_back(new_weight);
		}
	}
}

void finalize_loop_scope_states(ScopeNode* new_loop_scope_node,
								vector<int>& experiment_scope_context,
								vector<int>& experiment_node_context,
								vector<map<int, double>>& continue_input_state_weights,
								vector<map<int, double>>& continue_local_state_weights,
								vector<map<State*, double>>& continue_temp_state_weights,
								vector<map<int, double>>& halt_input_state_weights,
								vector<map<int, double>>& halt_local_state_weights,
								vector<map<State*, double>>& halt_temp_state_weights,
								map<pair<int, pair<bool,int>>, int>& input_scope_depths_mappings,
								map<pair<int, pair<bool,int>>, int>& output_scope_depths_mappings) {
	Scope* loop_scope = new_loop_scope_node->inner_scope;

	for (int c_index = 0; c_index < (int)experiment_scope_context.size()-1; c_index++) {
		int scope_depth = experiment_scope_context.size()-1 - c_index;
		for (map<int, double>::iterator continue_it = continue_input_state_weights[c_index].begin();
				continue_it != continue_input_state_weights[c_index].end(); continue_it++) {
			double continue_weight = continue_it->second;
			double halt_weight = halt_input_state_weights[c_index][continue_it->first];

			int new_state_index;
			map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
				.find({scope_depth, {false, continue_it->first}});
			if (input_it != input_scope_depths_mappings.end()) {
				new_state_index = input_it->second;

				/**
				 * - potential_loop input/output
				 */
				int new_loop_scope_state_index;
				for (int i_index = 0; i_index < (int)new_loop_scope_node->input_types.size(); i_index++) {
					if (new_loop_scope_node->input_outer_is_local[i_index] == false
							&& new_loop_scope_node->input_outer_indexes[i_index] == new_state_index) {
						new_loop_scope_state_index = new_loop_scope_node->input_inner_indexes[i_index];
						break;
					}
				}

				loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
				loop_scope->loop_continue_weights.push_back(continue_weight);
				loop_scope->loop_halt_weights.push_back(halt_weight);
			} else {
				add_new_input(experiment_scope_context,
							  experiment_node_context,
							  scope_depth,
							  false,
							  continue_it->first,
							  new_state_index);

				// no longer need to update input_scope_depths_mappings

				int new_loop_scope_state_index = loop_scope->num_input_states;
				loop_scope->num_input_states++;

				new_loop_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_loop_scope_node->input_inner_indexes.push_back(new_loop_scope_state_index);
				new_loop_scope_node->input_outer_is_local.push_back(false);
				new_loop_scope_node->input_outer_indexes.push_back(new_state_index);
				new_loop_scope_node->input_init_vals.push_back(0.0);
				new_loop_scope_node->input_init_index_vals.push_back(0.0);

				loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
				loop_scope->loop_continue_weights.push_back(continue_weight);
				loop_scope->loop_halt_weights.push_back(halt_weight);
			}
		}
	}
	{
		for (map<int, double>::iterator continue_it = continue_input_state_weights.back().begin();
				continue_it != continue_input_state_weights.back().end(); continue_it++) {
			double continue_weight = continue_it->second;
			double halt_weight = halt_input_state_weights.back()[continue_it->first];

			int new_loop_scope_state_index = -1;
			for (int i_index = 0; i_index < (int)new_loop_scope_node->input_types.size(); i_index++) {
				if (new_loop_scope_node->input_outer_is_local[i_index] == false
						&& new_loop_scope_node->input_outer_indexes[i_index] == continue_it->first) {
					new_loop_scope_state_index = new_loop_scope_node->input_inner_indexes[i_index];
					break;
				}
			}

			if (new_loop_scope_state_index == -1) {
				new_loop_scope_state_index = loop_scope->num_input_states;
				loop_scope->num_input_states++;

				new_loop_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_loop_scope_node->input_inner_indexes.push_back(new_loop_scope_state_index);
				new_loop_scope_node->input_outer_is_local.push_back(false);
				new_loop_scope_node->input_outer_indexes.push_back(continue_it->first);
				new_loop_scope_node->input_init_vals.push_back(0.0);
				new_loop_scope_node->input_init_index_vals.push_back(0.0);
			}

			loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
			loop_scope->loop_continue_weights.push_back(continue_weight);
			loop_scope->loop_halt_weights.push_back(halt_weight);
		}
	}

	for (int c_index = 0; c_index < (int)experiment_scope_context.size()-1; c_index++) {
		int scope_depth = experiment_scope_context.size()-1 - c_index;
		for (map<int, double>::iterator continue_it = continue_local_state_weights[c_index].begin();
				continue_it != continue_local_state_weights[c_index].end(); continue_it++) {
			double continue_weight = continue_it->second;
			double halt_weight = halt_local_state_weights[c_index][continue_it->first];

			int new_state_index;
			map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
				.find({scope_depth, {true, continue_it->first}});
			if (input_it != input_scope_depths_mappings.end()) {
				new_state_index = input_it->second;

				/**
				 * - potential_loop input/output
				 */
				int new_loop_scope_state_index;
				for (int i_index = 0; i_index < (int)new_loop_scope_node->input_types.size(); i_index++) {
					if (new_loop_scope_node->input_outer_is_local[i_index] == false
							&& new_loop_scope_node->input_outer_indexes[i_index] == new_state_index) {
						new_loop_scope_state_index = new_loop_scope_node->input_inner_indexes[i_index];
						break;
					}
				}

				loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
				loop_scope->loop_continue_weights.push_back(continue_weight);
				loop_scope->loop_halt_weights.push_back(halt_weight);
			} else {
				add_new_input(experiment_scope_context,
							  experiment_node_context,
							  scope_depth,
							  true,
							  continue_it->first,
							  new_state_index);

				// no longer need to update input_scope_depths_mappings

				int new_loop_scope_state_index = loop_scope->num_input_states;
				loop_scope->num_input_states++;

				new_loop_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_loop_scope_node->input_inner_indexes.push_back(new_loop_scope_state_index);
				new_loop_scope_node->input_outer_is_local.push_back(false);
				new_loop_scope_node->input_outer_indexes.push_back(new_state_index);
				new_loop_scope_node->input_init_vals.push_back(0.0);
				new_loop_scope_node->input_init_index_vals.push_back(0.0);

				loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
				loop_scope->loop_continue_weights.push_back(continue_weight);
				loop_scope->loop_halt_weights.push_back(halt_weight);
			}
		}
	}
	{
		for (map<int, double>::iterator continue_it = continue_local_state_weights.back().begin();
				continue_it != continue_local_state_weights.back().end(); continue_it++) {
			double continue_weight = continue_it->second;
			double halt_weight = halt_local_state_weights.back()[continue_it->first];

			int new_loop_scope_state_index = -1;
			for (int i_index = 0; i_index < (int)new_loop_scope_node->input_types.size(); i_index++) {
				if (new_loop_scope_node->input_outer_is_local[i_index] == true
						&& new_loop_scope_node->input_outer_indexes[i_index] == continue_it->first) {
					new_loop_scope_state_index = new_loop_scope_node->input_inner_indexes[i_index];
					break;
				}
			}

			if (new_loop_scope_state_index == -1) {
				new_loop_scope_state_index = loop_scope->num_input_states;
				loop_scope->num_input_states++;

				new_loop_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_loop_scope_node->input_inner_indexes.push_back(new_loop_scope_state_index);
				new_loop_scope_node->input_outer_is_local.push_back(true);
				new_loop_scope_node->input_outer_indexes.push_back(continue_it->first);
				new_loop_scope_node->input_init_vals.push_back(0.0);
				new_loop_scope_node->input_init_index_vals.push_back(0.0);
			}

			loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
			loop_scope->loop_continue_weights.push_back(continue_weight);
			loop_scope->loop_halt_weights.push_back(halt_weight);
		}
	}

	for (int c_index = 0; c_index < (int)experiment_scope_context.size()-1; c_index++) {
		int scope_depth = experiment_scope_context.size()-1 - c_index;
		for (map<State*, double>::iterator continue_it = continue_temp_state_weights[c_index].begin();
				continue_it != continue_temp_state_weights[c_index].end(); continue_it++) {
			double continue_weight = continue_it->second;
			double halt_weight = halt_temp_state_weights[c_index][continue_it->first];

			Scope* outer_scope = solution->scopes[experiment_scope_context[c_index]];

			int temp_state_index;
			for (int t_index = 0; t_index < (int)outer_scope->temp_states.size(); t_index++) {
				if (outer_scope->temp_states[t_index] == continue_it->first) {
					temp_state_index = t_index;
					break;
				}
			}

			if (outer_scope->temp_state_new_local_indexes[temp_state_index] != -1) {
				map<pair<int, pair<bool,int>>, int>::iterator input_it = input_scope_depths_mappings
					.find({scope_depth, {true, outer_scope->temp_state_new_local_indexes[temp_state_index]}});
				if (input_it != input_scope_depths_mappings.end()) {
					int new_state_index = input_it->second;

					/**
					 * - potential_loop input/output
					 */
					int new_loop_scope_state_index;
					for (int i_index = 0; i_index < (int)new_loop_scope_node->input_types.size(); i_index++) {
						if (new_loop_scope_node->input_outer_is_local[i_index] == false
								&& new_loop_scope_node->input_outer_indexes[i_index] == new_state_index) {
							new_loop_scope_state_index = new_loop_scope_node->input_inner_indexes[i_index];
							break;
						}
					}

					loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
					loop_scope->loop_continue_weights.push_back(continue_weight);
					loop_scope->loop_halt_weights.push_back(halt_weight);
				} else {
					int new_state_index;
					duplicate_temp_state_helper(experiment_scope_context,
												experiment_node_context,
												scope_depth,
												outer_scope->temp_state_new_local_indexes[temp_state_index],
												new_state_index);

					// no longer need to update input_scope_depths_mappings

					int new_loop_scope_state_index = loop_scope->num_input_states;
					loop_scope->num_input_states++;

					new_loop_scope_node->input_types.push_back(INPUT_TYPE_STATE);
					new_loop_scope_node->input_inner_indexes.push_back(new_loop_scope_state_index);
					new_loop_scope_node->input_outer_is_local.push_back(false);
					new_loop_scope_node->input_outer_indexes.push_back(new_state_index);
					new_loop_scope_node->input_init_vals.push_back(0.0);
					new_loop_scope_node->input_init_index_vals.push_back(0.0);

					loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
					loop_scope->loop_continue_weights.push_back(continue_weight);
					loop_scope->loop_halt_weights.push_back(halt_weight);
				}
			} else {
				int new_state_index;
				add_state(outer_scope,
						  temp_state_index,
						  experiment_scope_context,
						  experiment_node_context,
						  scope_depth,
						  new_state_index);

				// no longer need to update input_scope_depths_mappings

				int new_loop_scope_state_index = loop_scope->num_input_states;
				loop_scope->num_input_states++;

				new_loop_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_loop_scope_node->input_inner_indexes.push_back(new_loop_scope_state_index);
				new_loop_scope_node->input_outer_is_local.push_back(false);
				new_loop_scope_node->input_outer_indexes.push_back(new_state_index);
				new_loop_scope_node->input_init_vals.push_back(0.0);
				new_loop_scope_node->input_init_index_vals.push_back(0.0);

				loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
				loop_scope->loop_continue_weights.push_back(continue_weight);
				loop_scope->loop_halt_weights.push_back(halt_weight);
			}
		}
	}
	{
		for (map<State*, double>::iterator continue_it = continue_temp_state_weights.back().begin();
				continue_it != continue_temp_state_weights.back().end(); continue_it++) {
			double continue_weight = continue_it->second;
			double halt_weight = halt_temp_state_weights.back()[continue_it->first];

			Scope* outer_scope = solution->scopes[experiment_scope_context.back()];

			int temp_state_index;
			for (int t_index = 0; t_index < (int)outer_scope->temp_states.size(); t_index++) {
				if (outer_scope->temp_states[t_index] == continue_it->first) {
					temp_state_index = t_index;
					break;
				}
			}

			if (outer_scope->temp_state_new_local_indexes[temp_state_index] == -1) {
				int new_state_index;
				add_state(outer_scope,
						  temp_state_index,
						  experiment_scope_context,
						  experiment_node_context,
						  0,
						  new_state_index);
			}

			int outer_index = outer_scope->temp_state_new_local_indexes[temp_state_index];

			int new_loop_scope_state_index = -1;
			for (int i_index = 0; i_index < (int)new_loop_scope_node->input_types.size(); i_index++) {
				if (new_loop_scope_node->input_outer_is_local[i_index] == true
						&& new_loop_scope_node->input_outer_indexes[i_index] == outer_index) {
					new_loop_scope_state_index = new_loop_scope_node->input_inner_indexes[i_index];
					break;
				}
			}

			if (new_loop_scope_state_index == -1) {
				new_loop_scope_state_index = loop_scope->num_input_states;
				loop_scope->num_input_states++;

				new_loop_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_loop_scope_node->input_inner_indexes.push_back(new_loop_scope_state_index);
				new_loop_scope_node->input_outer_is_local.push_back(true);
				new_loop_scope_node->input_outer_indexes.push_back(outer_index);
				new_loop_scope_node->input_init_vals.push_back(0.0);
				new_loop_scope_node->input_init_index_vals.push_back(0.0);
			}

			loop_scope->loop_state_indexes.push_back(new_loop_scope_state_index);
			loop_scope->loop_continue_weights.push_back(continue_weight);
			loop_scope->loop_halt_weights.push_back(halt_weight);
		}
	}
}
