#include "helpers.h"

#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

pair<bool,AbstractNode*> end_node_helper(vector<Scope*>& scope_context,
										 vector<AbstractNode*>& node_context,
										 int curr_depth,
										 AbstractNode* curr_node,
										 map<AbstractNode*, pair<bool,AbstractNode*>>& node_mappings,
										 map<AbstractNode*, AbstractNode*>& new_node_reverse_mappings,
										 Scope* new_scope) {
	map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it = node_mappings.find(curr_node);
	if (it != node_mappings.end()) {
		return it->second;
	} else {
		pair<bool,AbstractNode*> mapping;
		if (curr_node == NULL) {
			/**
			 * - if curr_node is NULL and not already added, then it's off path
			 */
			mapping = {false, NULL};
		} else {
			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = end_node_helper(
						scope_context,
						node_context,
						curr_depth,
						action_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					if (next_mapping.first) {
						ActionNode* new_action_node = new ActionNode();

						new_action_node->parent = new_scope;
						new_action_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_action_node->id] = new_action_node;

						new_node_reverse_mappings[new_action_node] = action_node;

						new_action_node->action = action_node->action;

						if (next_mapping.second == NULL) {
							new_action_node->next_node_id = -1;
						} else {
							new_action_node->next_node_id = next_mapping.second->id;
						}
						new_action_node->next_node = next_mapping.second;

						mapping = {true, new_action_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = end_node_helper(
						scope_context,
						node_context,
						curr_depth,
						scope_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					if (next_mapping.first) {
						ScopeNode* new_scope_node = new ScopeNode();

						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] = new_scope_node;

						new_node_reverse_mappings[new_scope_node] = scope_node;

						new_scope_node->inner_scope = scope_node->inner_scope;

						if (next_mapping.second == NULL) {
							new_scope_node->next_node_id = -1;
						} else {
							new_scope_node->next_node_id = next_mapping.second->id;
						}
						new_scope_node->next_node = next_mapping.second;

						mapping = {true, new_scope_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					bool matches_context = true;
					if ((int)branch_node->branch_scope_context.size() > 1+curr_depth) {
						matches_context = false;
					} else {
						for (int c_index = 0; c_index < (int)branch_node->branch_scope_context.size()-1; c_index++) {
							if (branch_node->branch_scope_context[c_index] != scope_context[1+curr_depth - branch_node->branch_scope_context.size() + c_index]->id
									|| branch_node->branch_node_context[c_index] != node_context[1+curr_depth - branch_node->branch_scope_context.size() + c_index]->id) {
								matches_context = false;
								break;
							}
						}
					}

					if (matches_context) {
						if (branch_node->branch_is_pass_through) {
							pair<bool,AbstractNode*> branch_mapping = end_node_helper(
								scope_context,
								node_context,
								curr_depth,
								branch_node->branch_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							mapping = branch_mapping;
						} else {
							pair<bool,AbstractNode*> original_mapping = end_node_helper(
								scope_context,
								node_context,
								curr_depth,
								branch_node->original_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							pair<bool,AbstractNode*> branch_mapping = end_node_helper(
								scope_context,
								node_context,
								curr_depth,
								branch_node->branch_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							if (original_mapping.first && branch_mapping.first) {
								BranchNode* new_branch_node = new BranchNode();

								new_branch_node->parent = new_scope;
								new_branch_node->id = new_scope->node_counter;
								new_scope->node_counter++;
								new_scope->nodes[new_branch_node->id] = new_branch_node;

								new_node_reverse_mappings[new_branch_node] = branch_node;

								/**
								 * - leave context empty to always match
								 */

								new_branch_node->branch_is_pass_through = false;

								new_branch_node->original_score_mod = branch_node->original_score_mod;
								new_branch_node->branch_score_mod = branch_node->branch_score_mod;

								if (original_mapping.second == NULL) {
									new_branch_node->original_next_node_id = -1;
								} else {
									new_branch_node->original_next_node_id = original_mapping.second->id;
								}
								new_branch_node->original_next_node = original_mapping.second;
								if (branch_mapping.second == NULL) {
									new_branch_node->branch_next_node_id = -1;
								} else {
									new_branch_node->branch_next_node_id = branch_mapping.second->id;
								}
								new_branch_node->branch_next_node = branch_mapping.second;

								mapping = {true, new_branch_node};
							} else if (original_mapping.first) {
								mapping = original_mapping;
							} else if (branch_mapping.first) {
								mapping = branch_mapping;
							} else {
								mapping = {false, NULL};
							}
						}
					} else {
						pair<bool,AbstractNode*> original_mapping = end_node_helper(
							scope_context,
							node_context,
							curr_depth,
							branch_node->original_next_node,
							node_mappings,
							new_node_reverse_mappings,
							new_scope);

						mapping = original_mapping;
					}
				}

				break;
			case NODE_TYPE_EXIT:
				{
					ExitNode* exit_node = (ExitNode*)curr_node;
					if (exit_node->exit_depth > 0) {
						/**
						 * - if curr_node is ExitNode and not already added, then it's off path
						 */
						mapping = {false, NULL};
					} else {
						pair<bool,AbstractNode*> exit_mapping = end_node_helper(
							scope_context,
							node_context,
							curr_depth,
							exit_node->exit_node,
							node_mappings,
							new_node_reverse_mappings,
							new_scope);
						mapping = exit_mapping;
					}
				}

				break;
			}
		}

		node_mappings[curr_node] = mapping;
		return mapping;
	}
}

pair<bool,AbstractNode*> start_node_helper(vector<Scope*>& scope_context,
										   vector<AbstractNode*>& node_context,
										   int curr_depth,
										   int starting_depth,
										   AbstractNode* curr_node,
										   vector<map<AbstractNode*, pair<bool,AbstractNode*>>>& node_mappings,
										   vector<map<AbstractNode*, AbstractNode*>>& new_node_reverse_mappings,
										   Scope* new_scope) {
	map<AbstractNode*, pair<bool,AbstractNode*>>::iterator it = node_mappings[curr_depth].find(curr_node);
	if (it != node_mappings[curr_depth].end()) {
		return it->second;
	} else {
		pair<bool,AbstractNode*> mapping;
		if (curr_node == NULL) {
			if (curr_depth == starting_depth) {
				mapping = {false, NULL};
			} else {
				ScopeNode* outer_scope_node = (ScopeNode*)node_context[curr_depth-1];
				pair<bool,AbstractNode*> next_mapping = start_node_helper(
					scope_context,
					node_context,
					curr_depth-1,
					starting_depth,
					outer_scope_node->next_node,
					node_mappings,
					new_node_reverse_mappings,
					new_scope);
				mapping = next_mapping;
			}
		} else {
			switch (curr_node->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = start_node_helper(
						scope_context,
						node_context,
						curr_depth,
						starting_depth,
						action_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					if (next_mapping.first) {
						ActionNode* new_action_node = new ActionNode();

						new_action_node->parent = new_scope;
						new_action_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_action_node->id] = new_action_node;

						new_node_reverse_mappings[curr_depth][new_action_node] = action_node;

						new_action_node->action = action_node->action;

						if (next_mapping.second == NULL) {
							new_action_node->next_node_id = -1;
						} else {
							new_action_node->next_node_id = next_mapping.second->id;
						}
						new_action_node->next_node = next_mapping.second;

						mapping = {true, new_action_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)curr_node;

					pair<bool,AbstractNode*> next_mapping = start_node_helper(
						scope_context,
						node_context,
						curr_depth,
						starting_depth,
						scope_node->next_node,
						node_mappings,
						new_node_reverse_mappings,
						new_scope);

					if (next_mapping.first) {
						ScopeNode* new_scope_node = new ScopeNode();

						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] =new_scope_node;

						new_node_reverse_mappings[curr_depth][new_scope_node] = scope_node;

						new_scope_node->inner_scope = scope_node->inner_scope;

						if (next_mapping.second == NULL) {
							new_scope_node->next_node_id = -1;
						} else {
							new_scope_node->next_node_id = next_mapping.second->id;
						}
						new_scope_node->next_node = next_mapping.second;

						mapping = {true, new_scope_node};
					} else {
						mapping = {false, NULL};
					}
				}

				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)curr_node;

					bool matches_context = true;
					if ((int)branch_node->branch_scope_context.size() > 1+curr_depth) {
						matches_context = false;
					} else {
						for (int c_index = 0; c_index < (int)branch_node->branch_scope_context.size()-1; c_index++) {
							if (branch_node->branch_scope_context[c_index] != scope_context[1+curr_depth - branch_node->branch_scope_context.size() + c_index]->id
									|| branch_node->branch_node_context[c_index] != node_context[1+curr_depth - branch_node->branch_scope_context.size() + c_index]->id) {
								matches_context = false;
								break;
							}
						}
					}

					if (matches_context) {
						if (branch_node->branch_is_pass_through) {
							pair<bool,AbstractNode*> branch_mapping = start_node_helper(
								scope_context,
								node_context,
								curr_depth,
								starting_depth,
								branch_node->branch_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							mapping = branch_mapping;
						} else {
							pair<bool,AbstractNode*> original_mapping = start_node_helper(
								scope_context,
								node_context,
								curr_depth,
								starting_depth,
								branch_node->original_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							pair<bool,AbstractNode*> branch_mapping = start_node_helper(
								scope_context,
								node_context,
								curr_depth,
								starting_depth,
								branch_node->branch_next_node,
								node_mappings,
								new_node_reverse_mappings,
								new_scope);

							if (original_mapping.first && branch_mapping.first) {
								BranchNode* new_branch_node = new BranchNode();

								new_branch_node->parent = new_scope;
								new_branch_node->id = new_scope->node_counter;
								new_scope->node_counter++;
								new_scope->nodes[new_branch_node->id] = new_branch_node;

								new_node_reverse_mappings[curr_depth][new_branch_node] = branch_node;

								/**
								 * - can leave context empty to always match for explore
								 */

								new_branch_node->branch_is_pass_through = false;

								new_branch_node->original_score_mod = branch_node->original_score_mod;
								new_branch_node->branch_score_mod = branch_node->branch_score_mod;

								if (original_mapping.second == NULL) {
									new_branch_node->original_next_node_id = -1;
								} else {
									new_branch_node->original_next_node_id = original_mapping.second->id;
								}
								new_branch_node->original_next_node = original_mapping.second;
								if (branch_mapping.second == NULL) {
									new_branch_node->branch_next_node_id = -1;
								} else {
									new_branch_node->branch_next_node_id = branch_mapping.second->id;
								}
								new_branch_node->branch_next_node = branch_mapping.second;

								mapping = {true, new_branch_node};
							} else if (original_mapping.first) {
								mapping = original_mapping;
							} else if (branch_mapping.first) {
								mapping = branch_mapping;
							} else {
								mapping = {false, NULL};
							}
						}
					} else {
						pair<bool,AbstractNode*> original_mapping = start_node_helper(
							scope_context,
							node_context,
							curr_depth,
							starting_depth,
							branch_node->original_next_node,
							node_mappings,
							new_node_reverse_mappings,
							new_scope);

						mapping = original_mapping;
					}
				}

				break;
			case NODE_TYPE_EXIT:
				{
					ExitNode* exit_node = (ExitNode*)curr_node;

					if (exit_node->exit_depth > curr_depth - starting_depth) {
						mapping = {false, NULL};
					} else if (scope_context[curr_depth - exit_node->exit_depth]->id != exit_node->exit_node_parent_id) {
						mapping = {false, NULL};
					} else {
						pair<bool,AbstractNode*> exit_mapping = start_node_helper(
							scope_context,
							node_context,
							curr_depth - exit_node->exit_depth,
							starting_depth,
							exit_node->exit_node,
							node_mappings,
							new_node_reverse_mappings,
							new_scope);
						mapping = exit_mapping;
					}
				}

				break;
			}
		}

		node_mappings[curr_depth][curr_node] = mapping;
		return mapping;
	}
}

uniform_int_distribution<int> input_non_empty_distribution(0, 2);
uniform_int_distribution<int> input_type_distribution(0, 1);
uniform_int_distribution<int> init_distribution(-1, 1);
uniform_int_distribution<int> reuse_distribution(0, 3);
int start_new_state_helper(map<pair<bool,int>, int>& state_mappings,
						   bool is_local,
						   int index,
						   map<pair<bool,int>, int>& potential_new_state_mappings,
						   vector<int>& potential_to_final_states,
						   vector<int>& possible_input_scope_depths,
						   vector<int>& possible_input_outer_types,
						   vector<void*>& possible_input_outer_indexes,
						   Scope* new_scope,
						   PotentialScopeNode* new_potential_scope_node) {
	map<pair<bool,int>, int>::iterator it = state_mappings.find({is_local, index});
	if (it != state_mappings.end()) {
		return it->second;
	} else {
		int potential_index = potential_new_state_mappings[{is_local, index}];
		if (potential_to_final_states[potential_index] == -1) {
			if (is_local || input_non_empty_distribution(generator) == 0) {
				int new_state_index = new_scope->num_input_states;
				new_scope->num_input_states++;

				if (input_type_distribution(generator) == 0) {
					// state
					uniform_int_distribution<int> outer_distribution(0, possible_input_scope_depths.size()-1);
					int outer_index = outer_distribution(generator);

					new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
					new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
					new_potential_scope_node->input_scope_depths.push_back(possible_input_scope_depths[outer_index]);
					new_potential_scope_node->input_outer_types.push_back(possible_input_outer_types[outer_index]);
					new_potential_scope_node->input_outer_indexes.push_back(possible_input_outer_indexes[outer_index]);
					new_potential_scope_node->input_init_vals.push_back(0.0);
				} else {
					// constant
					new_potential_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
					new_potential_scope_node->input_scope_depths.push_back(-1);
					new_potential_scope_node->input_outer_types.push_back(-1);
					new_potential_scope_node->input_outer_indexes.push_back(NULL);
					new_potential_scope_node->input_init_vals.push_back(init_distribution(generator));
				}

				// no need to set potential_to_final_states

				state_mappings[{is_local, index}] = new_state_index;
				return new_state_index;
			} else {
				state_mappings[{is_local, index}] = -1;
				return -1;
			}
		} else {
			if (reuse_distribution(generator) != 0) {
				/**
				 * - simply reuse earliest
				 */
				new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
				new_potential_scope_node->input_inner_indexes.push_back(potential_to_final_states[potential_index]);
				new_potential_scope_node->input_scope_depths.push_back(
					new_potential_scope_node->output_scope_depths[potential_to_final_states[potential_index]]);
				new_potential_scope_node->input_outer_types.push_back(
					new_potential_scope_node->output_outer_types[potential_to_final_states[potential_index]]);
				new_potential_scope_node->input_outer_indexes.push_back(
					new_potential_scope_node->output_outer_indexes[potential_to_final_states[potential_index]]);
				new_potential_scope_node->input_init_vals.push_back(0.0);
			} else {
				if (input_type_distribution(generator) == 0) {
					// state
					uniform_int_distribution<int> outer_distribution(0, possible_input_scope_depths.size()-1);
					int outer_index = outer_distribution(generator);

					new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
					new_potential_scope_node->input_inner_indexes.push_back(potential_to_final_states[potential_index]);
					new_potential_scope_node->input_scope_depths.push_back(possible_input_scope_depths[outer_index]);
					new_potential_scope_node->input_outer_types.push_back(possible_input_outer_types[outer_index]);
					new_potential_scope_node->input_outer_indexes.push_back(possible_input_outer_indexes[outer_index]);
					new_potential_scope_node->input_init_vals.push_back(0.0);
				} else {
					// constant
					new_potential_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_potential_scope_node->input_inner_indexes.push_back(potential_to_final_states[potential_index]);
					new_potential_scope_node->input_scope_depths.push_back(-1);
					new_potential_scope_node->input_outer_types.push_back(-1);
					new_potential_scope_node->input_outer_indexes.push_back(NULL);
					new_potential_scope_node->input_init_vals.push_back(init_distribution(generator));
				}
			}

			state_mappings[{is_local, index}] = potential_to_final_states[potential_index];
			return potential_to_final_states[potential_index];
		}
	}
}

int end_new_state_helper(map<pair<bool,int>, int>& state_mappings,
						 bool is_local,
						 int index,
						 map<pair<bool,int>, int>& potential_new_state_mappings,
						 vector<int>& potential_to_final_states,
						 Scope* new_scope,
						 PotentialScopeNode* new_potential_scope_node) {
	map<pair<bool,int>, int>::iterator it = state_mappings.find({is_local, index});
	if (it != state_mappings.end()) {
		return it->second;
	} else {
		int potential_index = potential_new_state_mappings[{is_local, index}];
		if (potential_to_final_states[potential_index] == -1) {
			if (is_local) {
				int new_state_index = new_scope->num_input_states;
				new_scope->num_input_states++;

				new_potential_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
				new_potential_scope_node->input_scope_depths.push_back(-1);
				new_potential_scope_node->input_outer_types.push_back(-1);
				new_potential_scope_node->input_outer_indexes.push_back(NULL);
				new_potential_scope_node->input_init_vals.push_back(0.0);

				state_mappings[{is_local, index}] = new_state_index;
				return new_state_index;
			} else {
				state_mappings[{is_local, index}] = -1;
				return -1;
			}
		} else {
			new_potential_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
			new_potential_scope_node->input_inner_indexes.push_back(potential_to_final_states[potential_index]);
			new_potential_scope_node->input_scope_depths.push_back(-1);
			new_potential_scope_node->input_outer_types.push_back(-1);
			new_potential_scope_node->input_outer_indexes.push_back(NULL);
			new_potential_scope_node->input_init_vals.push_back(0.0);

			state_mappings[{is_local, index}] = potential_to_final_states[potential_index];
			return potential_to_final_states[potential_index];
		}
	}
}

PotentialScopeNode* create_scope(vector<ContextLayer>& context,
								 int explore_context_depth,
								 Scope* scope) {
	Scope* new_scope = new Scope();
	// don't set id/increment scope_counter until train
	new_scope->num_input_states = 0;
	new_scope->num_local_states = 0;
	new_scope->node_counter = 0;
	PotentialScopeNode* new_potential_scope_node = new PotentialScopeNode();
	new_potential_scope_node->scope = new_scope;

	// determine start and end

	vector<AbstractNode*> possible_nodes;
	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;

	vector<Scope*> scope_context{scope};
	vector<AbstractNode*> node_context{NULL};

	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	scope->random_activate(scope_context,
						   node_context,
						   exit_depth,
						   exit_node,
						   possible_nodes,
						   possible_scope_contexts,
						   possible_node_contexts);

	vector<Scope*> start_scope_context;
	vector<AbstractNode*> start_node_context;
	vector<Scope*> end_scope_context;
	vector<AbstractNode*> end_node_context;
	uniform_int_distribution<int> distribution(0, possible_nodes.size()-1);
	while (true) {
		int start_index = distribution(generator);
		int end_index = distribution(generator);
		if (start_index < end_index
				&& possible_nodes[start_index] != NULL
				&& possible_nodes[start_index]->type != NODE_TYPE_EXIT) {
			bool empty_path = true;
			for (int n_index = start_index; n_index < end_index; n_index++) {
				if (possible_nodes[n_index] != NULL) {
					if (possible_nodes[n_index]->type == NODE_TYPE_ACTION) {
						empty_path = false;
						break;
					} else if (possible_nodes[n_index]->type == NODE_TYPE_SCOPE) {
						if (possible_scope_contexts[n_index].size() == possible_scope_contexts[n_index+1].size()) {
							// same depth means that entire scope node included
							empty_path = false;
							break;
						}
					}
				}
			}
			if (!empty_path) {
				start_scope_context = possible_scope_contexts[start_index];
				start_node_context = possible_node_contexts[start_index];
				end_scope_context = possible_scope_contexts[end_index];
				end_node_context = possible_node_contexts[end_index];

				break;
			}
		}
	}

	// find and create needed nodes

	int starting_depth = 0;
	while (true) {
		if (starting_depth >= (int)start_scope_context.size()	// start can be empty
				|| starting_depth + 1 >= (int)end_scope_context.size()) {
			break;
		}

		if (start_scope_context[starting_depth] == end_scope_context[starting_depth]
				&& start_node_context[starting_depth] == end_node_context[starting_depth]) {
			starting_depth++;
		} else {
			break;
		}
	}

	AbstractNode* new_starting_node;

	vector<map<AbstractNode*, pair<bool,AbstractNode*>>> end_node_mappings(end_scope_context.size());
	vector<map<AbstractNode*, AbstractNode*>> end_new_node_reverse_mappings(end_scope_context.size());
	if ((int)start_scope_context.size() == starting_depth) {
		// start empty edge case
		{
			end_node_mappings.back()[end_node_context.back()] = {true, NULL};
			if (end_scope_context.back()->starting_node != end_node_context.back()) {
				end_node_helper(end_scope_context,
								end_node_context,
								(int)end_scope_context.size()-1,
								end_scope_context.back()->starting_node,
								end_node_mappings.back(),
								end_new_node_reverse_mappings.back(),
								new_scope);

				new_starting_node = end_node_mappings.back()[end_scope_context.back()->starting_node].second;
			}
		}
		for (int l_index = (int)end_scope_context.size()-2; l_index >= starting_depth; l_index--) {
			AbstractNode* next_layer_start = end_node_mappings[l_index + 1]
				[end_scope_context[l_index + 1]->starting_node].second;
			end_node_mappings[l_index][end_node_context[l_index]] = {true, next_layer_start};
			if (end_scope_context[l_index]->starting_node != end_node_context[l_index]) {
				end_node_helper(end_scope_context,
								end_node_context,
								l_index,
								end_scope_context[l_index]->starting_node,
								end_node_mappings[l_index],
								end_new_node_reverse_mappings[l_index],
								new_scope);

				new_starting_node = end_node_mappings[l_index][end_scope_context[l_index]->starting_node].second;
			}
		}
	} else if ((int)end_scope_context.size() > starting_depth + 1) {
		{
			end_node_mappings.back()[end_node_context.back()] = {true, NULL};
			if (end_scope_context.back()->starting_node != end_node_context.back()) {
				end_node_helper(end_scope_context,
								end_node_context,
								(int)end_scope_context.size()-1,
								end_scope_context.back()->starting_node,
								end_node_mappings.back(),
								end_new_node_reverse_mappings.back(),
								new_scope);

				new_starting_node = end_node_mappings.back()[end_scope_context.back()->starting_node].second;
			}
		}
		for (int l_index = (int)end_scope_context.size()-2; l_index > starting_depth; l_index--) {
			AbstractNode* next_layer_start = end_node_mappings[l_index + 1]
				[end_scope_context[l_index + 1]->starting_node].second;
			end_node_mappings[l_index][end_node_context[l_index]] = {true, next_layer_start};
			if (end_scope_context[l_index]->starting_node != end_node_context[l_index]) {
				end_node_helper(end_scope_context,
								end_node_context,
								l_index,
								end_scope_context[l_index]->starting_node,
								end_node_mappings[l_index],
								end_new_node_reverse_mappings[l_index],
								new_scope);

				new_starting_node = end_node_mappings[l_index][end_scope_context[l_index]->starting_node].second;
			}
		}
	}

	vector<map<AbstractNode*, pair<bool,AbstractNode*>>> start_node_mappings(start_scope_context.size());
	vector<map<AbstractNode*, AbstractNode*>> start_new_node_reverse_mappings(start_scope_context.size());
	if ((int)start_scope_context.size() > starting_depth) {
		AbstractNode* end_node;
		if ((int)end_node_mappings.size() > starting_depth + 1) {
			end_node = end_node_mappings[starting_depth + 1]
				[end_scope_context[starting_depth + 1]->starting_node].second;
		} else {
			end_node = NULL;
		}
		start_node_mappings[starting_depth][end_node_context[starting_depth]] = {true, end_node};
		start_node_helper(start_scope_context,
						  start_node_context,
						  (int)start_scope_context.size()-1,
						  starting_depth,
						  start_node_context.back(),
						  start_node_mappings,
						  start_new_node_reverse_mappings,
						  new_scope);

		new_starting_node = start_node_mappings.back()[start_node_context.back()].second;
	}

	ActionNode* new_noop_action_node = new ActionNode();
	new_noop_action_node->parent = new_scope;
	new_noop_action_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_noop_action_node->action = Action(ACTION_NOOP);
	new_noop_action_node->next_node_id = new_starting_node->id;
	new_noop_action_node->next_node = new_starting_node;
	new_scope->nodes[new_noop_action_node->id] = new_noop_action_node;

	new_scope->starting_node_id = new_noop_action_node->id;
	new_scope->starting_node = new_noop_action_node;

	// gather potential states

	int potential_new_state_counter = 0;

	vector<map<pair<bool,int>, int>> start_potential_new_state_mappings(start_scope_context.size());
	for (int l_index = (int)start_new_node_reverse_mappings.size()-1; l_index >= starting_depth; l_index--) {
		for (map<AbstractNode*, AbstractNode*>::iterator node_it = start_new_node_reverse_mappings[l_index].begin();
				node_it != start_new_node_reverse_mappings[l_index].end(); node_it++) {
			if (node_it->second->type == NODE_TYPE_ACTION) {
				ActionNode* original_action_node = (ActionNode*)node_it->second;
				for (int n_index = 0; n_index < (int)original_action_node->state_is_local.size(); n_index++) {
					map<pair<bool,int>, int>::iterator state_it = start_potential_new_state_mappings[l_index]
						.find({original_action_node->state_is_local[n_index], original_action_node->state_indexes[n_index]});
					if (state_it == start_potential_new_state_mappings[l_index].end()) {
						start_potential_new_state_mappings[l_index][{original_action_node->state_is_local[n_index], original_action_node->state_indexes[n_index]}] = potential_new_state_counter;
						potential_new_state_counter++;
					}
				}
			} else if (node_it->second->type == NODE_TYPE_SCOPE) {
				ScopeNode* original_scope_node = (ScopeNode*)node_it->second;
				for (int i_index = 0; i_index < (int)original_scope_node->input_types.size(); i_index++) {
					if (original_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
						map<pair<bool,int>, int>::iterator state_it = start_potential_new_state_mappings[l_index]
							.find({original_scope_node->input_outer_is_local[i_index], original_scope_node->input_outer_indexes[i_index]});
						if (state_it == start_potential_new_state_mappings[l_index].end()) {
							start_potential_new_state_mappings[l_index][{original_scope_node->input_outer_is_local[i_index], original_scope_node->input_outer_indexes[i_index]}] = potential_new_state_counter;
							potential_new_state_counter++;
						}
					}
				}
				for (int o_index = 0; o_index < (int)original_scope_node->output_inner_indexes.size(); o_index++) {
					map<pair<bool,int>, int>::iterator state_it = start_potential_new_state_mappings[l_index]
						.find({original_scope_node->output_outer_is_local[o_index], original_scope_node->output_outer_indexes[o_index]});
					if (state_it == start_potential_new_state_mappings[l_index].end()) {
						start_potential_new_state_mappings[l_index][{original_scope_node->output_outer_is_local[o_index], original_scope_node->output_outer_indexes[o_index]}] = potential_new_state_counter;
						potential_new_state_counter++;
					}
				}
			} else {
				BranchNode* original_branch_node = (BranchNode*)node_it->second;
				for (int s_index = 0; s_index < (int)original_branch_node->decision_state_is_local.size(); s_index++) {
					map<pair<bool,int>, int>::iterator state_it = start_potential_new_state_mappings[l_index]
						.find({original_branch_node->decision_state_is_local[s_index], original_branch_node->decision_state_indexes[s_index]});
					if (state_it == start_potential_new_state_mappings[l_index].end()) {
						start_potential_new_state_mappings[l_index][{original_branch_node->decision_state_is_local[s_index], original_branch_node->decision_state_indexes[s_index]}] = potential_new_state_counter;
						potential_new_state_counter++;
					}
				}
			}
		}

		if (l_index != starting_depth) {
			ScopeNode* outer_scope_node = (ScopeNode*)start_node_context[l_index-1];
			for (int o_index = 0; o_index < (int)outer_scope_node->output_inner_indexes.size(); o_index++) {
				map<pair<bool,int>, int>::iterator it = start_potential_new_state_mappings[l_index]
					.find({false, outer_scope_node->output_inner_indexes[o_index]});
				if (it != start_potential_new_state_mappings[l_index].end()) {
					start_potential_new_state_mappings[l_index-1][{outer_scope_node->output_outer_is_local[o_index], outer_scope_node->output_outer_indexes[o_index]}] = it->second;
				}
			}
		}
	}

	vector<map<pair<bool,int>, int>> end_potential_new_state_mappings(end_scope_context.size());
	// start from starting_depth, even through may be empty
	for (int l_index = starting_depth; l_index < (int)end_scope_context.size(); l_index++) {
		if (l_index > starting_depth) {
			ScopeNode* outer_scope_node = (ScopeNode*)end_node_context[l_index-1];
			if (l_index == starting_depth+1 && (int)start_scope_context.size() != starting_depth) {
				for (int i_index = 0; i_index < (int)outer_scope_node->input_types.size(); i_index++) {
					map<pair<bool,int>, int>::iterator it = start_potential_new_state_mappings[starting_depth]
						.find({outer_scope_node->input_outer_is_local[i_index], outer_scope_node->input_outer_indexes[i_index]});
					if (it != start_potential_new_state_mappings[starting_depth].end()) {
						end_potential_new_state_mappings[starting_depth+1][{false, outer_scope_node->input_inner_indexes[i_index]}] = it->second;
					}
				}
			} else {
				for (int i_index = 0; i_index < (int)outer_scope_node->input_types.size(); i_index++) {
					map<pair<bool,int>, int>::iterator it = end_potential_new_state_mappings[l_index-1]
						.find({outer_scope_node->input_outer_is_local[i_index], outer_scope_node->input_outer_indexes[i_index]});
					if (it != end_potential_new_state_mappings[l_index-1].end()) {
						end_potential_new_state_mappings[l_index][{false, outer_scope_node->input_inner_indexes[i_index]}] = it->second;
					}
				}
			}
		}

		for (map<AbstractNode*, AbstractNode*>::iterator node_it = end_new_node_reverse_mappings[l_index].begin();
				node_it != end_new_node_reverse_mappings[l_index].end(); node_it++) {
			if (node_it->second->type == NODE_TYPE_ACTION) {
				ActionNode* original_action_node = (ActionNode*)node_it->second;
				for (int n_index = 0; n_index < (int)original_action_node->state_is_local.size(); n_index++) {
					map<pair<bool,int>, int>::iterator state_it = end_potential_new_state_mappings[l_index]
						.find({original_action_node->state_is_local[n_index], original_action_node->state_indexes[n_index]});
					if (state_it == end_potential_new_state_mappings[l_index].end()) {
						end_potential_new_state_mappings[l_index][{original_action_node->state_is_local[n_index], original_action_node->state_indexes[n_index]}] = potential_new_state_counter;
						potential_new_state_counter++;
					}
				}
			} else if (node_it->second->type == NODE_TYPE_SCOPE) {
				ScopeNode* original_scope_node = (ScopeNode*)node_it->second;
				for (int i_index = 0; i_index < (int)original_scope_node->input_types.size(); i_index++) {
					map<pair<bool,int>, int>::iterator state_it = end_potential_new_state_mappings[l_index]
						.find({original_scope_node->input_outer_is_local[i_index], original_scope_node->input_outer_indexes[i_index]});
					if (state_it == end_potential_new_state_mappings[l_index].end()) {
						end_potential_new_state_mappings[l_index][{original_scope_node->input_outer_is_local[i_index], original_scope_node->input_outer_indexes[i_index]}] = potential_new_state_counter;
						potential_new_state_counter++;
					}
				}
				for (int o_index = 0; o_index < (int)original_scope_node->output_inner_indexes.size(); o_index++) {
					map<pair<bool,int>, int>::iterator state_it = end_potential_new_state_mappings[l_index]
						.find({original_scope_node->output_outer_is_local[o_index], original_scope_node->output_outer_indexes[o_index]});
					if (state_it == end_potential_new_state_mappings[l_index].end()) {
						end_potential_new_state_mappings[l_index][{original_scope_node->output_outer_is_local[o_index], original_scope_node->output_outer_indexes[o_index]}] = potential_new_state_counter;
						potential_new_state_counter++;
					}
				}
			} else {
				BranchNode* original_branch_node = (BranchNode*)node_it->second;
				for (int s_index = 0; s_index < (int)original_branch_node->decision_state_is_local.size(); s_index++) {
					map<pair<bool,int>, int>::iterator state_it = end_potential_new_state_mappings[l_index]
						.find({original_branch_node->decision_state_is_local[s_index], original_branch_node->decision_state_indexes[s_index]});
					if (state_it == end_potential_new_state_mappings[l_index].end()) {
						end_potential_new_state_mappings[l_index][{original_branch_node->decision_state_is_local[s_index], original_branch_node->decision_state_indexes[s_index]}] = potential_new_state_counter;
						potential_new_state_counter++;
					}
				}
			}
		}
	}

	if (potential_new_state_counter > 0) {
		// assign outputs

		vector<int> possible_output_scope_depths;
		vector<int> possible_output_outer_types;
		vector<void*> possible_output_outer_indexes;
		uniform_int_distribution<int> output_temp_distribution(0, 4);
		{
			Scope* scope = context.back().scope;

			/**
			 * - check for OuterExperiment edge case
			 */
			if (scope != NULL) {
				for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
						it != context.back().input_state_vals.end(); it++) {
					possible_output_scope_depths.push_back(0);
					possible_output_outer_types.push_back(OUTER_TYPE_INPUT);
					possible_output_outer_indexes.push_back((void*)((long)it->first));
				}

				for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
					possible_output_scope_depths.push_back(0);
					possible_output_outer_types.push_back(OUTER_TYPE_LOCAL);
					possible_output_outer_indexes.push_back((void*)((long)s_index));
				}

				for (map<State*, StateStatus>::iterator it = context.back().temp_state_vals.begin();
						it != context.back().temp_state_vals.end(); it++) {
					if (output_temp_distribution(generator) == 0) {
						possible_output_scope_depths.push_back(0);
						possible_output_outer_types.push_back(OUTER_TYPE_TEMP);
						possible_output_outer_indexes.push_back(it->first);
					}
				}
			}
		}
		for (int c_index = 1; c_index < explore_context_depth; c_index++) {
			Scope* scope = context[context.size()-1 - c_index].scope;
			ScopeNode* scope_node = context[context.size()-1 - c_index].node;

			for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
					it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
				bool passed_out = false;
				for (int o_index = 0; o_index < (int)scope_node->output_inner_indexes.size(); o_index++) {
					if (scope_node->output_outer_is_local[o_index] == false
							&& scope_node->output_outer_indexes[o_index] == it->first) {
						passed_out = true;
						break;
					}
				}

				if (!passed_out) {
					possible_output_scope_depths.push_back(c_index);
					possible_output_outer_types.push_back(OUTER_TYPE_INPUT);
					possible_output_outer_indexes.push_back((void*)((long)it->first));
				}
			}

			for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
				bool passed_out = false;
				for (int o_index = 0; o_index < (int)scope_node->output_inner_indexes.size(); o_index++) {
					if (scope_node->output_outer_is_local[o_index] == true
							&& scope_node->output_outer_indexes[o_index] == s_index) {
						passed_out = true;
						break;
					}
				}

				if (!passed_out) {
					possible_output_scope_depths.push_back(c_index);
					possible_output_outer_types.push_back(OUTER_TYPE_LOCAL);
					possible_output_outer_indexes.push_back((void*)((long)s_index));
				}
			}

			for (map<State*, StateStatus>::iterator it = context[context.size()-1 - c_index].temp_state_vals.begin();
					it != context[context.size()-1 - c_index].temp_state_vals.end(); it++) {
				if (output_temp_distribution(generator) == 0) {
					possible_output_scope_depths.push_back(c_index);
					possible_output_outer_types.push_back(OUTER_TYPE_TEMP);
					possible_output_outer_indexes.push_back(it->first);
				}
			}
		}

		vector<int> potential_to_final_states(potential_new_state_counter, -1);
		if (possible_output_scope_depths.size() > 0) {
			uniform_int_distribution<int> output_consider_distribution(0, possible_output_scope_depths.size()-1);
			int num_outputs_to_consider = output_consider_distribution(generator);

			uniform_int_distribution<int> output_non_empty_distribution(0, 2);
			uniform_int_distribution<int> potential_distribution(0, potential_new_state_counter-1);
			for (int c_index = 0; c_index < num_outputs_to_consider; c_index++) {
				uniform_int_distribution<int> outer_distribution(0, possible_output_scope_depths.size()-1);
				int outer_index = outer_distribution(generator);

				if (output_non_empty_distribution(generator) == 0) {
					int potential_index = potential_distribution(generator);
					if (potential_to_final_states[potential_index] == -1) {
						potential_to_final_states[potential_index] = new_scope->num_input_states;
						new_scope->num_input_states++;
					}

					new_potential_scope_node->output_inner_indexes.push_back(potential_to_final_states[potential_index]);
					new_potential_scope_node->output_scope_depths.push_back(possible_output_scope_depths[outer_index]);
					new_potential_scope_node->output_outer_types.push_back(possible_output_outer_types[outer_index]);
					new_potential_scope_node->output_outer_indexes.push_back(possible_output_outer_indexes[outer_index]);
				}

				possible_output_scope_depths.erase(possible_output_scope_depths.begin() + outer_index);
				possible_output_outer_types.erase(possible_output_outer_types.begin() + outer_index);
				possible_output_outer_indexes.erase(possible_output_outer_indexes.begin() + outer_index);
			}
		}

		// add state

		vector<int> possible_input_scope_depths;
		vector<int> possible_input_outer_types;
		vector<void*> possible_input_outer_indexes;
		uniform_int_distribution<int> input_temp_distribution(0, 2);
		{
			for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
					it != context.back().input_state_vals.end(); it++) {
				possible_input_scope_depths.push_back(0);
				possible_input_outer_types.push_back(OUTER_TYPE_INPUT);
				possible_input_outer_indexes.push_back((void*)((long)it->first));
			}

			for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
					it != context.back().local_state_vals.end(); it++) {
				possible_input_scope_depths.push_back(0);
				possible_input_outer_types.push_back(OUTER_TYPE_LOCAL);
				possible_input_outer_indexes.push_back((void*)((long)it->first));
			}

			for (map<State*, StateStatus>::iterator it = context.back().temp_state_vals.begin();
					it != context.back().temp_state_vals.end(); it++) {
				if (input_temp_distribution(generator) == 0) {
					possible_input_scope_depths.push_back(0);
					possible_input_outer_types.push_back(OUTER_TYPE_TEMP);
					possible_input_outer_indexes.push_back(it->first);
				}
			}
		}
		for (int c_index = 1; c_index < explore_context_depth; c_index++) {
			ScopeNode* scope_node = context[context.size()-1 - c_index].node;

			for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
					it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
				bool passed_down = false;
				for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
					if (scope_node->input_outer_is_local[i_index] == false
							&& scope_node->input_outer_indexes[i_index] == it->first) {
						passed_down = true;
						break;
					}
				}

				if (!passed_down) {
					possible_input_scope_depths.push_back(c_index);
					possible_input_outer_types.push_back(OUTER_TYPE_INPUT);
					possible_input_outer_indexes.push_back((void*)((long)it->first));
				}
			}

			for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].local_state_vals.begin();
					it != context[context.size()-1 - c_index].local_state_vals.end(); it++) {
				bool passed_down = false;
				for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
					if (scope_node->input_outer_is_local[i_index] == true
							&& scope_node->input_outer_indexes[i_index] == it->first) {
						passed_down = true;
						break;
					}
				}

				if (!passed_down) {
					possible_input_scope_depths.push_back(c_index);
					possible_input_outer_types.push_back(OUTER_TYPE_LOCAL);
					possible_input_outer_indexes.push_back((void*)((long)it->first));
				}
			}

			for (map<State*, StateStatus>::iterator it = context[context.size()-1 - c_index].temp_state_vals.begin();
					it != context[context.size()-1 - c_index].temp_state_vals.end(); it++) {
				if (input_temp_distribution(generator) == 0) {
					possible_input_scope_depths.push_back(c_index);
					possible_input_outer_types.push_back(OUTER_TYPE_TEMP);
					possible_input_outer_indexes.push_back(it->first);
				}
			}
		}

		if (possible_input_scope_depths.size() > 0) {
			/**
			 * - don't have to worry about duplicate scopes/nodes in context
			 *   - new nodes/states created for each copy
			 */
			vector<map<pair<bool,int>, int>> start_state_mappings(start_scope_context.size());
			for (int l_index = (int)start_new_node_reverse_mappings.size()-1; l_index >= starting_depth; l_index--) {
				for (map<AbstractNode*, AbstractNode*>::iterator node_it = start_new_node_reverse_mappings[l_index].begin();
						node_it != start_new_node_reverse_mappings[l_index].end(); node_it++) {
					if (node_it->first->type == NODE_TYPE_ACTION) {
						ActionNode* new_action_node = (ActionNode*)node_it->first;
						ActionNode* original_action_node = (ActionNode*)node_it->second;

						vector<int> obs_index_mapping(original_action_node->state_is_local.size(), -1);
						for (int n_index = 0; n_index < (int)original_action_node->state_is_local.size(); n_index++) {
							int new_state_index = start_new_state_helper(start_state_mappings[l_index],
																		 original_action_node->state_is_local[n_index],
																		 original_action_node->state_indexes[n_index],
																		 start_potential_new_state_mappings[l_index],
																		 potential_to_final_states,
																		 possible_input_scope_depths,
																		 possible_input_outer_types,
																		 possible_input_outer_indexes,
																		 new_scope,
																		 new_potential_scope_node);
							if (new_state_index != -1) {
								if (original_action_node->state_obs_indexes[n_index] == -1) {
									obs_index_mapping[n_index] = (int)new_action_node->state_is_local.size();

									new_action_node->state_is_local.push_back(false);
									new_action_node->state_indexes.push_back(new_state_index);
									new_action_node->state_obs_indexes.push_back(-1);
									new_action_node->state_defs.push_back(original_action_node->state_defs[n_index]);
									new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[n_index]);
								} else {
									if (obs_index_mapping[original_action_node->state_obs_indexes[n_index]] != -1) {
										obs_index_mapping[n_index] = (int)new_action_node->state_is_local.size();

										new_action_node->state_is_local.push_back(false);
										new_action_node->state_indexes.push_back(new_state_index);
										new_action_node->state_obs_indexes.push_back(obs_index_mapping[original_action_node->state_obs_indexes[n_index]]);
										new_action_node->state_defs.push_back(original_action_node->state_defs[n_index]);
										new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[n_index]);
									}
								}
							}
						}
					} else if (node_it->first->type == NODE_TYPE_SCOPE) {
						ScopeNode* new_scope_node = (ScopeNode*)node_it->first;
						ScopeNode* original_scope_node = (ScopeNode*)node_it->second;

						for (int i_index = 0; i_index < (int)original_scope_node->input_types.size(); i_index++) {
							if (original_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
								int new_state_index = start_new_state_helper(start_state_mappings[l_index],
																			 original_scope_node->input_outer_is_local[i_index],
																			 original_scope_node->input_outer_indexes[i_index],
																			 start_potential_new_state_mappings[l_index],
																			 potential_to_final_states,
																			 possible_input_scope_depths,
																			 possible_input_outer_types,
																			 possible_input_outer_indexes,
																			 new_scope,
																			 new_potential_scope_node);

								if (new_state_index != -1) {
									new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
									new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
									new_scope_node->input_outer_is_local.push_back(false);
									new_scope_node->input_outer_indexes.push_back(new_state_index);
									new_scope_node->input_init_vals.push_back(0.0);
								}
							} else {
								new_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
								new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
								new_scope_node->input_outer_is_local.push_back(false);
								new_scope_node->input_outer_indexes.push_back(-1);
								new_scope_node->input_init_vals.push_back(original_scope_node->input_init_vals[i_index]);
							}
						}

						for (int o_index = 0; o_index < (int)original_scope_node->output_inner_indexes.size(); o_index++) {
							int new_state_index = start_new_state_helper(start_state_mappings[l_index],
																		 original_scope_node->output_outer_is_local[o_index],
																		 original_scope_node->output_outer_indexes[o_index],
																		 start_potential_new_state_mappings[l_index],
																		 potential_to_final_states,
																		 possible_input_scope_depths,
																		 possible_input_outer_types,
																		 possible_input_outer_indexes,
																		 new_scope,
																		 new_potential_scope_node);

							if (new_state_index != -1) {
								new_scope_node->output_inner_indexes.push_back(original_scope_node->output_inner_indexes[o_index]);
								new_scope_node->output_outer_is_local.push_back(false);
								new_scope_node->output_outer_indexes.push_back(new_state_index);
							}
						}
					} else {
						BranchNode* new_branch_node = (BranchNode*)node_it->first;
						BranchNode* original_branch_node = (BranchNode*)node_it->second;

						for (int s_index = 0; s_index < (int)original_branch_node->decision_state_is_local.size(); s_index++) {
							int new_state_index = start_new_state_helper(start_state_mappings[l_index],
																		 original_branch_node->decision_state_is_local[s_index],
																		 original_branch_node->decision_state_indexes[s_index],
																		 start_potential_new_state_mappings[l_index],
																		 potential_to_final_states,
																		 possible_input_scope_depths,
																		 possible_input_outer_types,
																		 possible_input_outer_indexes,
																		 new_scope,
																		 new_potential_scope_node);

							if (new_state_index != -1) {
								new_branch_node->decision_state_is_local.push_back(false);
								new_branch_node->decision_state_indexes.push_back(new_state_index);
								new_branch_node->decision_original_weights.push_back(original_branch_node->decision_original_weights[s_index]);
								new_branch_node->decision_branch_weights.push_back(original_branch_node->decision_branch_weights[s_index]);
							}
						}
					}
				}

				if (l_index != starting_depth) {
					ScopeNode* outer_scope_node = (ScopeNode*)start_node_context[l_index-1];
					for (int o_index = 0; o_index < (int)outer_scope_node->output_inner_indexes.size(); o_index++) {
						map<pair<bool,int>, int>::iterator it = start_state_mappings[l_index]
							.find({false, outer_scope_node->output_inner_indexes[o_index]});
						if (it != start_state_mappings[l_index].end()) {
							start_state_mappings[l_index-1][{outer_scope_node->output_outer_is_local[o_index], outer_scope_node->output_outer_indexes[o_index]}] = it->second;
						}
					}
				}
			}

			vector<map<pair<bool,int>, int>> end_state_mappings(end_scope_context.size());
			// start from starting_depth, even through may be empty
			for (int l_index = starting_depth; l_index < (int)end_new_node_reverse_mappings.size(); l_index++) {
				if (l_index > starting_depth) {
					ScopeNode* outer_scope_node = (ScopeNode*)end_node_context[l_index-1];
					if (l_index == starting_depth+1 && (int)start_scope_context.size() != starting_depth) {
						for (int i_index = 0; i_index < (int)outer_scope_node->input_types.size(); i_index++) {
							map<pair<bool,int>, int>::iterator it = start_state_mappings[starting_depth]
								.find({outer_scope_node->input_outer_is_local[i_index], outer_scope_node->input_outer_indexes[i_index]});
							if (it != start_state_mappings[starting_depth].end()) {
								end_state_mappings[starting_depth+1][{false, outer_scope_node->input_inner_indexes[i_index]}] = it->second;
							}
						}
					} else {
						for (int i_index = 0; i_index < (int)outer_scope_node->input_types.size(); i_index++) {
							map<pair<bool,int>, int>::iterator it = end_state_mappings[l_index-1]
								.find({outer_scope_node->input_outer_is_local[i_index], outer_scope_node->input_outer_indexes[i_index]});
							if (it != end_state_mappings[l_index-1].end()) {
								end_state_mappings[l_index][{false, outer_scope_node->input_inner_indexes[i_index]}] = it->second;
							}
						}
					}
				}

				for (map<AbstractNode*, AbstractNode*>::iterator node_it = end_new_node_reverse_mappings[l_index].begin();
						node_it != end_new_node_reverse_mappings[l_index].end(); node_it++) {
					if (node_it->second->type == NODE_TYPE_ACTION) {
						ActionNode* new_action_node = (ActionNode*)node_it->first;
						ActionNode* original_action_node = (ActionNode*)node_it->second;

						vector<int> obs_index_mapping(original_action_node->state_is_local.size(), -1);
						for (int n_index = 0; n_index < (int)original_action_node->state_is_local.size(); n_index++) {
							int new_state_index = end_new_state_helper(end_state_mappings[l_index],
																	   original_action_node->state_is_local[n_index],
																	   original_action_node->state_indexes[n_index],
																	   end_potential_new_state_mappings[l_index],
																	   potential_to_final_states,
																	   new_scope,
																	   new_potential_scope_node);

							if (new_state_index != -1) {
								if (original_action_node->state_obs_indexes[n_index] == -1) {
									obs_index_mapping[n_index] = (int)new_action_node->state_is_local.size();

									new_action_node->state_is_local.push_back(false);
									new_action_node->state_indexes.push_back(new_state_index);
									new_action_node->state_obs_indexes.push_back(-1);
									new_action_node->state_defs.push_back(original_action_node->state_defs[n_index]);
									new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[n_index]);
								} else {
									if (obs_index_mapping[original_action_node->state_obs_indexes[n_index]] != -1) {
										obs_index_mapping[n_index] = (int)new_action_node->state_is_local.size();

										new_action_node->state_is_local.push_back(false);
										new_action_node->state_indexes.push_back(new_state_index);
										new_action_node->state_obs_indexes.push_back(obs_index_mapping[original_action_node->state_obs_indexes[n_index]]);
										new_action_node->state_defs.push_back(original_action_node->state_defs[n_index]);
										new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[n_index]);
									}
								}
							}
						}
					} else if (node_it->second->type == NODE_TYPE_SCOPE) {
						ScopeNode* new_scope_node = (ScopeNode*)node_it->first;
						ScopeNode* original_scope_node = (ScopeNode*)node_it->second;

						for (int i_index = 0; i_index < (int)original_scope_node->input_types.size(); i_index++) {
							if (original_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
								int new_state_index = end_new_state_helper(end_state_mappings[l_index],
																		   original_scope_node->input_outer_is_local[i_index],
																		   original_scope_node->input_outer_indexes[i_index],
																		   end_potential_new_state_mappings[l_index],
																		   potential_to_final_states,
																		   new_scope,
																		   new_potential_scope_node);

								if (new_state_index != -1) {
									new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
									new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
									new_scope_node->input_outer_is_local.push_back(false);
									new_scope_node->input_outer_indexes.push_back(new_state_index);
									new_scope_node->input_init_vals.push_back(0.0);
								}
							} else {
								new_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
								new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
								new_scope_node->input_outer_is_local.push_back(false);
								new_scope_node->input_outer_indexes.push_back(-1);
								new_scope_node->input_init_vals.push_back(original_scope_node->input_init_vals[i_index]);
							}
						}

						for (int o_index = 0; o_index < (int)original_scope_node->output_inner_indexes.size(); o_index++) {
							int new_state_index = end_new_state_helper(end_state_mappings[l_index],
																	   original_scope_node->output_outer_is_local[o_index],
																	   original_scope_node->output_outer_indexes[o_index],
																	   end_potential_new_state_mappings[l_index],
																	   potential_to_final_states,
																	   new_scope,
																	   new_potential_scope_node);

							if (new_state_index != -1) {
								new_scope_node->output_inner_indexes.push_back(original_scope_node->output_inner_indexes[o_index]);
								new_scope_node->output_outer_is_local.push_back(false);
								new_scope_node->output_outer_indexes.push_back(new_state_index);
							}
						}
					} else {
						BranchNode* new_branch_node = (BranchNode*)node_it->first;
						BranchNode* original_branch_node = (BranchNode*)node_it->second;

						for (int s_index = 0; s_index < (int)original_branch_node->decision_state_is_local.size(); s_index++) {
							int new_state_index = end_new_state_helper(end_state_mappings[l_index],
																	   original_branch_node->decision_state_is_local[s_index],
																	   original_branch_node->decision_state_indexes[s_index],
																	   end_potential_new_state_mappings[l_index],
																	   potential_to_final_states,
																	   new_scope,
																	   new_potential_scope_node);

							if (new_state_index != -1) {
								new_branch_node->decision_state_is_local.push_back(false);
								new_branch_node->decision_state_indexes.push_back(new_state_index);
								new_branch_node->decision_original_weights.push_back(original_branch_node->decision_original_weights[s_index]);
								new_branch_node->decision_branch_weights.push_back(original_branch_node->decision_branch_weights[s_index]);
							}
						}
					}
				}
			}
		}
	}

	return new_potential_scope_node;
}
