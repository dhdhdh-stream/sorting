#include "pass_through_experiment.h"

#include <iostream>
#include <sstream>

#include "action_node.h"
#include "constants.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void PassThroughExperiment::clean() {
	this->node_context->experiment = NULL;
}

void PassThroughExperiment::add(SolutionWrapper* wrapper) {
	stringstream ss;
	ss << "PassThroughExperiment" << "; ";
	ss << "this->scope_context->id: " << this->scope_context->id << "; ";
	ss << "this->node_context->id: " << this->node_context->id << "; ";
	ss << "this->is_branch: " << this->is_branch << "; ";
	ss << "new explore path:";
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ss << " " << this->actions[s_index];
		} else {
			ss << " E" << this->scopes[s_index]->id;
		}
	}
	ss << "; ";

	if (this->exit_next_node == NULL) {
		ss << "this->exit_next_node->id: " << -1 << "; ";
	} else {
		ss << "this->exit_next_node->id: " << this->exit_next_node->id << "; ";
	}

	wrapper->solution->improvement_history.push_back(calc_new_score());
	wrapper->solution->change_history.push_back(ss.str());

	cout << ss.str() << endl;

	if (this->new_scope != NULL) {
		wrapper->solution->scopes.push_back(this->new_scope);
		this->new_scope->id = (int)wrapper->solution->scopes.size()-1;

		clean_scope(this->new_scope,
					wrapper);

		recursive_add_child(scope_context,
							wrapper,
							this->new_scope);

		this->new_scope = NULL;
	}

	vector<AbstractNode*> new_nodes;
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = scope_context;
			new_action_node->id = scope_context->node_counter;
			scope_context->node_counter++;
			scope_context->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = this->actions[s_index];

			new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = scope_context;
			new_scope_node->id = scope_context->node_counter;
			scope_context->node_counter++;
			scope_context->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->scope = this->scopes[s_index];

			new_nodes.push_back(new_scope_node);
		}
	}

	int starting_node_id;
	AbstractNode* starting_node;
	if (this->step_types.size() == 0) {
		if (this->exit_next_node == NULL) {
			starting_node_id = -1;
			starting_node = NULL;
		} else {
			starting_node_id = this->exit_next_node->id;
			starting_node = this->exit_next_node;
		}
	} else {
		starting_node_id = new_nodes[0]->id;
		starting_node = new_nodes[0];
	}

	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;

			if (start_node->next_node != NULL) {
				for (int a_index = 0; a_index < (int)start_node->next_node->ancestor_ids.size(); a_index++) {
					if (start_node->next_node->ancestor_ids[a_index] == start_node->id) {
						start_node->next_node->ancestor_ids.erase(
							start_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
			}

			start_node->next_node_id = starting_node_id;
			start_node->next_node = starting_node;

			if (starting_node != NULL) {
				starting_node->ancestor_ids.push_back(start_node->id);
			}
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			if (action_node->next_node != NULL) {
				for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
					if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
						action_node->next_node->ancestor_ids.erase(
							action_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
			}

			action_node->next_node_id = starting_node_id;
			action_node->next_node = starting_node;

			if (starting_node != NULL) {
				starting_node->ancestor_ids.push_back(action_node->id);
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			if (scope_node->next_node != NULL) {
				for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
					if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
						scope_node->next_node->ancestor_ids.erase(
							scope_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
			}

			scope_node->next_node_id = starting_node_id;
			scope_node->next_node = starting_node;

			if (starting_node != NULL) {
				starting_node->ancestor_ids.push_back(scope_node->id);
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				if (branch_node->branch_next_node != NULL) {
					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(
								branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
				}

				branch_node->branch_next_node_id = starting_node_id;
				branch_node->branch_next_node = starting_node;
			} else {
				if (branch_node->original_next_node != NULL) {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
				}

				branch_node->original_next_node_id = starting_node_id;
				branch_node->original_next_node = starting_node;
			}

			starting_node->ancestor_ids.push_back(branch_node->id);
		}
		break;
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)this->node_context;

			if (branch_end_node->next_node != NULL) {
				for (int a_index = 0; a_index < (int)branch_end_node->next_node->ancestor_ids.size(); a_index++) {
					if (branch_end_node->next_node->ancestor_ids[a_index] == branch_end_node->id) {
						branch_end_node->next_node->ancestor_ids.erase(
							branch_end_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
			}

			branch_end_node->next_node_id = starting_node_id;
			branch_end_node->next_node = starting_node;

			if (starting_node != NULL) {
				starting_node->ancestor_ids.push_back(branch_end_node->id);
			}
		}
		break;
	}

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			if (this->exit_next_node == NULL) {
				next_node_id = -1;
				next_node = NULL;
			} else {
				next_node_id = this->exit_next_node->id;
				next_node = this->exit_next_node;
			}
		} else {
			next_node_id = new_nodes[n_index+1]->id;
			next_node = new_nodes[n_index+1];
		}

		switch (new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)new_nodes[n_index];
				action_node->next_node_id = next_node_id;
				action_node->next_node = next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)new_nodes[n_index];
				scope_node->next_node_id = next_node_id;
				scope_node->next_node = next_node;
			}
			break;
		}

		if (next_node != NULL) {
			next_node->ancestor_ids.push_back(new_nodes[n_index]->id);
		}
	}
}

double PassThroughExperiment::calc_new_score() {
	return this->total_sum_scores / this->total_count;
}
