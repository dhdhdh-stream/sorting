#include "obs_node.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

void ObsNode::damage_step(SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	uniform_int_distribution<int> damage_distribution(0, 99);
	if (this->parent->id != -1 && damage_distribution(generator) == 0) {		
		history->is_damage = true;

		AbstractNode* starting_node = this->next_node;
		vector<AbstractNode*> possible_exits;
		this->parent->random_exit_activate(
			starting_node,
			possible_exits);

		geometric_distribution<int> exit_distribution(0.1);
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		AbstractNode* exit_next_node = possible_exits[random_index];
		history->end_damage = exit_next_node;

		uniform_int_distribution<int> new_scope_distribution(0, 3);
		if (new_scope_distribution(generator) == 0) {
			Scope* parent_scope;
			create_new_scope(this->parent,
			 				 wrapper,
			 				 history->damage_new_scope,
			 				 parent_scope);
		}
		if (history->damage_new_scope != NULL) {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->id = this->parent->damage_index;
			this->parent->damage_index++;
			new_scope_node->scope = history->damage_new_scope;
			history->damage_nodes.push_back(new_scope_node);
		} else {
			bool is_in_place;
			if (history->end_damage == this->next_node) {
				is_in_place = true;
			} else {
				is_in_place = false;
			}

			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			/**
			 * - num_steps less than exit length on average to reduce solution size
			 */
			if (is_in_place) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			vector<int> possible_child_indexes;
			for (int c_index = 0; c_index < (int)this->parent->child_scopes.size(); c_index++) {
				if (this->parent->child_scopes[c_index]->nodes.size() > 1) {
					possible_child_indexes.push_back(c_index);
				}
			}
			uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				bool is_scope = false;
				if (possible_child_indexes.size() > 0) {
					if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
						uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
						if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
							is_scope = true;
						}
					} else {
						uniform_int_distribution<int> scope_distribution(0, 1);
						if (scope_distribution(generator) == 0) {
							is_scope = true;
						}
					}
				}
				if (is_scope) {
					ScopeNode* new_scope_node = new ScopeNode();
					new_scope_node->id = this->parent->damage_index;
					this->parent->damage_index++;
					int child_index = possible_child_indexes[child_index_distribution(generator)];
					new_scope_node->scope = this->parent->child_scopes[child_index];
					history->damage_nodes.push_back(new_scope_node);
				} else {
					ActionNode* new_action_node = new ActionNode();
					new_action_node->id = this->parent->damage_index;
					this->parent->damage_index++;
					uniform_int_distribution<int> action_distribution(0, 6);
					new_action_node->action = action_distribution(generator);
					history->damage_nodes.push_back(new_action_node);
				}
			}
		}

		if (history->damage_nodes.size() == 0) {
			wrapper->node_context.back() = exit_next_node;
		} else {
			for (int n_index = 0; n_index < (int)history->damage_nodes.size()-1; n_index++) {
				switch (history->damage_nodes[n_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)history->damage_nodes[n_index];
						action_node->next_node = history->damage_nodes[n_index+1];
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)history->damage_nodes[n_index];
						scope_node->next_node = history->damage_nodes[n_index+1];
					}
					break;
				}
			}
			switch (history->damage_nodes.back()->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)history->damage_nodes.back();
					action_node->next_node = exit_next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)history->damage_nodes.back();
					scope_node->next_node = exit_next_node;
				}
				break;
			}

			wrapper->node_context.back() = history->damage_nodes[0];
		}
	} else {
		wrapper->node_context.back() = this->next_node;
	}
}
