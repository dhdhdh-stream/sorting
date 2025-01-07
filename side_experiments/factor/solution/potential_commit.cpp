#include "potential_commit.h"

using namespace std;

void PotentialCommit::activate(AbstractNode*& curr_node,
							   Problem* problem,
							   vector<ContextLayer>& context,
							   RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			problem->perform_action(this->actions[s_index]);
		} else {
			context.back().node_id = -1;

			this->scopes[s_index]->activate(problem,
				context,
				run_helper);
		}

		run_helper.num_actions += 2;
	}

	curr_node = this->exit_next_node;
}

void PotentialCommit::finalize() {
	Scope* parent_scope = this->node_context->parent;

	this->node_context->was_commit = true;

	vector<AbstractNode*> new_nodes;
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = parent_scope;
			new_action_node->id = parent_scope->node_counter;
			parent_scope->node_counter++;
			parent_scope->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = this->actions[s_index];

			new_action_node->average_instances_per_run = this->node_context->average_instances_per_run;

			new_action_node->was_commit = true;

			new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = parent_scope;
			new_scope_node->id = parent_scope->node_counter;
			parent_scope->node_counter++;
			parent_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->scope = this->scopes[s_index];

			new_scope_node->average_instances_per_run = this->node_context->average_instances_per_run;

			new_scope_node->was_commit = true;

			new_nodes.push_back(new_scope_node);
		}
	}

	solution->was_commit = true;

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
		ObsNode* new_ending_node = new ObsNode();
		new_ending_node->parent = parent_scope;
		new_ending_node->id = parent_scope->node_counter;
		parent_scope->node_counter++;

		for (map<int, AbstractNode*>::iterator it = parent_scope->nodes.begin();
				it != parent_scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->next_node == NULL) {
					obs_node->next_node_id = new_ending_node->id;
					obs_node->next_node = new_ending_node;

					new_ending_node->ancestors.push_back(obs_node);

					break;
				}
			}
		}

		parent_scope->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		new_ending_node->average_instances_per_run = this->node_context->average_instances_per_run;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			for (int a_index = 0; a_index < (int)action_node->next_node->ancestors.size(); a_index++) {
				if (action_node->next_node->ancestors[a_index] == action_node) {
					action_node->next_node->ancestors.erase(
						action_node->next_node->ancestors.begin() + a_index);
					break;
				}
			}

			action_node->next_node_id = new_nodes[0]->id;
			action_node->next_node = new_nodes[0];

			new_nodes[0]->ancestors.push_back(action_node);
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			for (int a_index = 0; a_index < (int)scope_node->next_node->ancestors.size(); a_index++) {
				if (scope_node->next_node->ancestors[a_index] == action_node) {
					scope_node->next_node->ancestors.erase(
						scope_node->next_node->ancestors.begin() + a_index);
					break;
				}
			}

			scope_node->next_node_id = new_nodes[0]->id;
			scope_node->next_node = new_nodes[0];

			new_nodes[0]->ancestors.push_back(scope_node);
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestors.size(); a_index++) {
					if (branch_node->branch_next_node->ancestors[a_index] == branch_node) {
						branch_node->branch_next_node->ancestors.erase(
							branch_node->branch_next_node->ancestors.begin() + a_index);
						break;
					}
				}

				branch_node->branch_next_node_id = new_nodes[0]->id;
				branch_node->branch_next_node = new_nodes[0];
			} else {
				for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestors.size(); a_index++) {
					if (branch_node->original_next_node->ancestors[a_index] == branch_node) {
						branch_node->original_next_node->ancestors.erase(
							branch_node->original_next_node->ancestors.begin() + a_index);
						break;
					}
				}

				branch_node->original_next_node_id = new_nodes[0]->id;
				branch_node->original_next_node = new_nodes[0];
			}

			new_nodes[0]->ancestors.push_back(branch_node);
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;

			if (obs_node->next_node != NULL) {
				for (int a_index = 0; a_index < (int)obs_node->next_node->ancestors.size(); a_index++) {
					if (obs_node->next_node->ancestors[a_index] == obs_node) {
						obs_node->next_node->ancestors.erase(
							obs_node->next_node->ancestors.begin() + a_index);
						break;
					}
				}
			}

			obs_node->next_node_id = new_nodes[0]->id;
			obs_node->next_node = new_nodes[0];

			new_nodes[0]->ancestors.push_back(obs_node);
		}
		break;
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (s_index == (int)this->step_types.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			next_node_id = new_nodes[s_index+1]->id;
			next_node = new_nodes[s_index+1];
		}

		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)new_nodes[s_index];
			action_node->next_node_id = next_node_id;
			action_node->next_node = next_node;
		} else {
			ScopeNode* scope_node = (ScopeNode*)new_nodes[s_index];
			scope_node->next_node_id = next_node_id;
			scope_node->next_node = next_node;
		}

		next_node->ancestors.push_back(new_nodes[s_index]);
	}
}
