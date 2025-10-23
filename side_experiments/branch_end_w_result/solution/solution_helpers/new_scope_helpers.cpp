#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "start_node.h"

using namespace std;

const int NEW_SCOPE_MIN_NUM_NODES = 3;
const int CREATE_NEW_SCOPE_NUM_TRIES = 100;

void ancestor_helper(AbstractNode* curr_node,
					 set<AbstractNode*>& ancestors) {
	for (int a_index = 0; a_index < (int)curr_node->ancestor_ids.size(); a_index++) {
		AbstractNode* next_node = curr_node->parent->nodes[curr_node->ancestor_ids[a_index]];
		set<AbstractNode*>::iterator it = ancestors.find(next_node);
		if (it == ancestors.end()) {
			ancestors.insert(next_node);

			ancestor_helper(next_node,
							ancestors);
		}
	}
}

void children_helper(AbstractNode* curr_node,
					 set<AbstractNode*>& children) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)curr_node;
			set<AbstractNode*>::iterator it = children.find(action_node->next_node);
			if (it == children.end()) {
				children.insert(action_node->next_node);

				children_helper(action_node->next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)curr_node;
			set<AbstractNode*>::iterator it = children.find(scope_node->next_node);
			if (it == children.end()) {
				children.insert(scope_node->next_node);

				children_helper(scope_node->next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)curr_node;
			set<AbstractNode*>::iterator original_it = children.find(branch_node->original_next_node);
			if (original_it == children.end()) {
				children.insert(branch_node->original_next_node);

				children_helper(branch_node->original_next_node,
								children);
			}
			set<AbstractNode*>::iterator branch_it = children.find(branch_node->branch_next_node);
			if (branch_it == children.end()) {
				children.insert(branch_node->branch_next_node);

				children_helper(branch_node->branch_next_node,
								children);
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)curr_node;
			if (obs_node->next_node != NULL) {
				set<AbstractNode*>::iterator it = children.find(obs_node->next_node);
				if (it == children.end()) {
					children.insert(obs_node->next_node);

					children_helper(obs_node->next_node,
									children);
				}
			}
		}
		break;
	}
}

/**
 * TODO: also create new scopes from external
 */
Scope* create_new_scope(Scope* scope_context) {
	if (scope_context->nodes.size() < NEW_SCOPE_MIN_NODES) {
		return NULL;
	}

	for (int t_index = 0; t_index < CREATE_NEW_SCOPE_NUM_TRIES; t_index++) {
		uniform_int_distribution<int> node_distribution(1, scope_context->nodes.size()-1);
		AbstractNode* potential_start_node = next(scope_context->nodes.begin(), node_distribution(generator))->second;

		AbstractNode* starting_node;
		switch (potential_start_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)potential_start_node;
				starting_node = start_node->next_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)potential_start_node;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)potential_start_node;
				starting_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)potential_start_node;
				uniform_int_distribution<int> branch_distribution(0, 1);
				if (branch_distribution(generator) == 0) {
					starting_node = branch_node->branch_next_node;
				} else {
					starting_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)potential_start_node;
				starting_node = obs_node->next_node;
			}
			break;
		}
		vector<AbstractNode*> possible_exits;
		scope_context->random_exit_activate(
			starting_node,
			possible_exits);
		if (possible_exits.size() < 2) {
			continue;
		}
		uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-2);
		/**
		 * - don't include possible_exits end
		 */
		AbstractNode* potential_end_node = possible_exits[exit_distribution(generator)];

		set<AbstractNode*> children;
		children_helper(potential_start_node,
						children);

		set<AbstractNode*> ancestors;
		ancestor_helper(potential_end_node,
						ancestors);

		set<AbstractNode*> potential_included_nodes;
		for (set<AbstractNode*>::iterator it = children.begin(); it != children.end(); it++) {
			if (ancestors.find(*it) != ancestors.end()) {
				potential_included_nodes.insert(*it);
			}
		}
		potential_included_nodes.insert(potential_start_node);
		potential_included_nodes.insert(potential_end_node);

		int num_meaningful_nodes = 0;
		for (set<AbstractNode*>::iterator it = potential_included_nodes.begin();
				it != potential_included_nodes.end(); it++) {
			switch ((*it)->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_SCOPE:
				num_meaningful_nodes++;
				break;
			}
		}
		if (num_meaningful_nodes >= NEW_SCOPE_MIN_NUM_NODES) {
			Scope* new_scope = new Scope();
			new_scope->id = -1;

			new_scope->node_counter = 0;

			StartNode* starting_node = new StartNode();
			starting_node->parent = new_scope;
			starting_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[starting_node->id] = starting_node;

			map<AbstractNode*, AbstractNode*> node_mappings;
			for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
					node_it != potential_included_nodes.end(); node_it++) {
				switch ((*node_it)->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* original_action_node = (ActionNode*)(*node_it);

						ActionNode* new_action_node = new ActionNode();
						new_action_node->parent = new_scope;
						new_action_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_action_node->id] = new_action_node;

						new_action_node->action = original_action_node->action;

						node_mappings[original_action_node] = new_action_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* original_scope_node = (ScopeNode*)(*node_it);

						ScopeNode* new_scope_node = new ScopeNode();
						new_scope_node->parent = new_scope;
						new_scope_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_scope_node->id] = new_scope_node;

						new_scope_node->scope = original_scope_node->scope;

						node_mappings[original_scope_node] = new_scope_node;
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* original_obs_node = (ObsNode*)(*node_it);

						ObsNode* new_obs_node = new ObsNode();
						new_obs_node->parent = new_scope;
						new_obs_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_obs_node->id] = new_obs_node;

						node_mappings[original_obs_node] = new_obs_node;
					}
					break;
				}
			}
			for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
					node_it != potential_included_nodes.end(); node_it++) {
				if ((*node_it)->type == NODE_TYPE_BRANCH) {
					BranchNode* original_branch_node = (BranchNode*)(*node_it);

					map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings.find(original_branch_node->original_next_node);
					map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings.find(original_branch_node->branch_next_node);
					if (original_it != node_mappings.end()
							&& branch_it != node_mappings.end()) {
						BranchNode* new_branch_node = new BranchNode();
						new_branch_node->parent = new_scope;
						new_branch_node->id = new_scope->node_counter;
						new_scope->node_counter++;
						new_scope->nodes[new_branch_node->id] = new_branch_node;

						node_mappings[original_branch_node] = new_branch_node;
					} else if (original_it != node_mappings.end()) {
						node_mappings[original_branch_node] = original_it->second;
					} else if (branch_it != node_mappings.end()) {
						node_mappings[original_branch_node] = branch_it->second;
					}
				}
			}

			starting_node->next_node_id = node_mappings[potential_start_node]->id;
			starting_node->next_node = node_mappings[potential_start_node];

			starting_node->next_node->ancestor_ids.push_back(starting_node->id);

			ObsNode* new_ending_node = new ObsNode();
			new_ending_node->parent = new_scope;
			new_ending_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_ending_node->id] = new_ending_node;
			new_ending_node->next_node_id = -1;
			new_ending_node->next_node = NULL;

			for (int f_index = 0; f_index < (int)scope_context->factors.size(); f_index++) {
				Factor* original_factor = scope_context->factors[f_index];
				Factor* new_factor = new Factor();

				new_factor->network = new Network(original_factor->network);
				for (int i_index = (int)original_factor->inputs.size()-1; i_index >= 0; i_index--) {
					if (original_factor->inputs[i_index].factor_index != -1
							&& original_factor->inputs[i_index].scope_context.size() == 1) {
						Input new_input = original_factor->inputs[i_index];
						new_input.scope_context[0] = new_scope;
						new_factor->inputs.insert(new_factor->inputs.begin(), new_input);
					} else {
						AbstractNode* original_input_node = scope_context->nodes[
							original_factor->inputs[i_index].node_context[0]];
						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings.find(original_input_node);
						/**
						 * - TODO: handle inputs for removed BranchNodes better
						 */
						if (it == node_mappings.end()
								|| original_input_node->type != it->second->type) {
							new_factor->network->remove_input(i_index);
						} else {
							Input new_input = original_factor->inputs[i_index];
							new_input.scope_context[0] = new_scope;
							new_input.node_context[0] = it->second->id;
							new_factor->inputs.insert(new_factor->inputs.begin(), new_input);
						}
					}
				}

				new_factor->is_meaningful = original_factor->is_meaningful;

				new_scope->factors.push_back(new_factor);

				new_factor->link(f_index);
			}

			for (map<AbstractNode*, AbstractNode*>::iterator node_it = node_mappings.begin();
					node_it != node_mappings.end(); node_it++) {
				switch (node_it->first->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* original_action_node = (ActionNode*)node_it->first;
						ActionNode* new_action_node = (ActionNode*)node_mappings[original_action_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_action_node->next_node);
						if (it == node_mappings.end()) {
							new_action_node->next_node_id = new_ending_node->id;
							new_action_node->next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_action_node->id);
						} else {
							new_action_node->next_node_id = it->second->id;
							new_action_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_action_node->id);
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* original_scope_node = (ScopeNode*)node_it->first;
						ScopeNode* new_scope_node = (ScopeNode*)node_mappings[original_scope_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_scope_node->next_node);
						if (it == node_mappings.end()) {
							new_scope_node->next_node_id = new_ending_node->id;
							new_scope_node->next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_scope_node->id);
						} else {
							new_scope_node->next_node_id = it->second->id;
							new_scope_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_scope_node->id);
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					if (node_it->first->type == node_it->second->type) {
						BranchNode* original_branch_node = (BranchNode*)node_it->first;
						BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

						new_branch_node->constant = original_branch_node->constant;

						for (int i_index = 0; i_index < (int)original_branch_node->inputs.size(); i_index++) {
							if (original_branch_node->inputs[i_index].factor_index != -1
									&& original_branch_node->inputs[i_index].scope_context.size() == 1) {
								Input new_input = original_branch_node->inputs[i_index];
								new_input.scope_context[0] = new_scope;
								new_branch_node->inputs.push_back(new_input);
								new_branch_node->input_averages.push_back(
									original_branch_node->input_averages[i_index]);
								new_branch_node->input_standard_deviations.push_back(
									original_branch_node->input_standard_deviations[i_index]);
								new_branch_node->weights.push_back(
									original_branch_node->weights[i_index]);
							} else {
								AbstractNode* original_input_node = scope_context->nodes[
									original_branch_node->inputs[i_index].node_context[0]];
								map<AbstractNode*, AbstractNode*>::iterator input_it = node_mappings
									.find(original_input_node);
								/**
								 * - TODO: handle inputs for removed BranchNodes better
								 */
								if (input_it != node_mappings.end()
										&& original_input_node->type == input_it->second->type) {
									Input new_input = original_branch_node->inputs[i_index];
									new_input.scope_context[0] = new_scope;
									new_input.node_context[0] = input_it->second->id;
									new_branch_node->inputs.push_back(new_input);
									new_branch_node->input_averages.push_back(
										original_branch_node->input_averages[i_index]);
									new_branch_node->input_standard_deviations.push_back(
										original_branch_node->input_standard_deviations[i_index]);
									new_branch_node->weights.push_back(
										original_branch_node->weights[i_index]);
								}
							}
						}

						map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
							.find(original_branch_node->original_next_node);
						if (original_it == node_mappings.end()) {
							new_branch_node->original_next_node_id = new_ending_node->id;
							new_branch_node->original_next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_branch_node->id);
						} else {
							new_branch_node->original_next_node_id = original_it->second->id;
							new_branch_node->original_next_node = original_it->second;

							original_it->second->ancestor_ids.push_back(new_branch_node->id);
						}
						map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
							.find(original_branch_node->branch_next_node);
						if (branch_it == node_mappings.end()) {
							new_branch_node->branch_next_node_id = new_ending_node->id;
							new_branch_node->branch_next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_branch_node->id);
						} else {
							new_branch_node->branch_next_node_id = branch_it->second->id;
							new_branch_node->branch_next_node = branch_it->second;

							branch_it->second->ancestor_ids.push_back(new_branch_node->id);
						}

						map<AbstractNode*, AbstractNode*>::iterator end_it = node_mappings
							.find(original_branch_node->branch_end_node);
						new_branch_node->branch_end_node_id = end_it->second->id;
						new_branch_node->branch_end_node = (ObsNode*)end_it->second;
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* original_obs_node = (ObsNode*)node_it->first;
						ObsNode* new_obs_node = (ObsNode*)node_mappings[original_obs_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_obs_node->next_node);
						if (it == node_mappings.end()) {
							new_obs_node->next_node_id = new_ending_node->id;
							new_obs_node->next_node = new_ending_node;

							new_ending_node->ancestor_ids.push_back(new_obs_node->id);
						} else {
							new_obs_node->next_node_id = it->second->id;
							new_obs_node->next_node = it->second;

							it->second->ancestor_ids.push_back(new_obs_node->id);
						}

						if (original_obs_node->branch_start_node == NULL
								|| original_obs_node == potential_start_node) {
							new_obs_node->branch_start_node_id = -1;
							new_obs_node->branch_start_node = NULL;
						} else {
							map<AbstractNode*, AbstractNode*>::iterator start_it = node_mappings
								.find(original_obs_node->branch_start_node);
							new_obs_node->branch_start_node_id = start_it->second->id;
							new_obs_node->branch_start_node = (BranchNode*)start_it->second;
						}
					}
					break;
				}
			}

			return new_scope;
		}
	}

	return NULL;
}
