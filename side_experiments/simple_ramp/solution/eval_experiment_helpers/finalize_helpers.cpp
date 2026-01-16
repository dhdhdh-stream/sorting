#include "eval_experiment.h"

#include <iostream>
#include <sstream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope) {
	curr_parent->child_scopes.push_back(new_scope);

	for (map<int, Scope*>::iterator it = wrapper->solution->scopes.begin();
			it != wrapper->solution->scopes.end(); it++) {
		bool is_needed = false;
		bool is_added = false;
		for (int c_index = 0; c_index < (int)it->second->child_scopes.size(); c_index++) {
			if (it->second->child_scopes[c_index] == curr_parent) {
				is_needed = true;
			}

			if (it->second->child_scopes[c_index] == new_scope) {
				is_added = true;
			}
		}

		if (is_needed && !is_added) {
			recursive_add_child(it->second,
								wrapper,
								new_scope);
		}
	}
}

void EvalExperiment::add(SolutionWrapper* wrapper) {
	stringstream ss;
	ss << "this->node_context->parent->id: " << this->node_context->parent->id << "; ";
	ss << "this->node_context->id: " << this->node_context->id << "; ";
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

	double score_average = wrapper->solution->sum_scores / (double)wrapper->solution->existing_scope_histories.size();
	cout << "score_average: " << score_average << endl;
	wrapper->solution->curr_score = score_average;

	wrapper->solution->improvement_history.push_back(score_average);
	wrapper->solution->change_history.push_back(ss.str());

	cout << ss.str() << endl;

	Scope* scope_context = this->node_context->parent;

	if (this->new_scope != NULL) {
		this->new_scope->id = wrapper->scope_counter;
		wrapper->scope_counter++;
		wrapper->solution->scopes[this->new_scope->id] = this->new_scope;

		clean_scope(this->new_scope,
					wrapper);

		this->new_scope->child_scopes = scope_context->child_scopes;
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

	ObsNode* new_ending_node = NULL;

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
		new_ending_node = new ObsNode();
		new_ending_node->parent = scope_context;
		new_ending_node->id = scope_context->node_counter;
		scope_context->node_counter++;

		for (map<int, AbstractNode*>::iterator it = scope_context->nodes.begin();
				it != scope_context->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->next_node == NULL) {
					obs_node->next_node_id = new_ending_node->id;
					obs_node->next_node = new_ending_node;

					new_ending_node->ancestor_ids.push_back(obs_node->id);

					break;
				}
			}
		}

		scope_context->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = scope_context;
	new_branch_node->id = scope_context->node_counter;
	scope_context->node_counter++;
	scope_context->nodes[new_branch_node->id] = new_branch_node;

	if (this->node_context->next_node == NULL) {
		if (new_ending_node != NULL) {
			new_ending_node->ancestor_ids.push_back(new_branch_node->id);

			new_branch_node->original_next_node_id = new_ending_node->id;
			new_branch_node->original_next_node = new_ending_node;
		} else {
			new_ending_node = new ObsNode();
			new_ending_node->parent = scope_context;
			new_ending_node->id = scope_context->node_counter;
			scope_context->node_counter++;

			for (map<int, AbstractNode*>::iterator it = scope_context->nodes.begin();
					it != scope_context->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_OBS) {
					ObsNode* p_obs_node = (ObsNode*)it->second;
					if (p_obs_node->next_node == NULL) {
						p_obs_node->next_node_id = new_ending_node->id;
						p_obs_node->next_node = new_ending_node;

						new_ending_node->ancestor_ids.push_back(p_obs_node->id);

						break;
					}
				}
			}

			scope_context->nodes[new_ending_node->id] = new_ending_node;

			new_ending_node->next_node_id = -1;
			new_ending_node->next_node = NULL;

			new_ending_node->ancestor_ids.push_back(new_branch_node->id);

			new_branch_node->original_next_node_id = new_ending_node->id;
			new_branch_node->original_next_node = new_ending_node;
		}
	} else {
		for (int a_index = 0; a_index < (int)this->node_context->next_node->ancestor_ids.size(); a_index++) {
			if (this->node_context->next_node->ancestor_ids[a_index] == this->node_context->id) {
				this->node_context->next_node->ancestor_ids.erase(
					this->node_context->next_node->ancestor_ids.begin() + a_index);
				break;
			}
		}
		this->node_context->next_node->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->original_next_node_id = this->node_context->next_node_id;
		new_branch_node->original_next_node = this->node_context->next_node;
	}

	if (this->step_types.size() == 0) {
		exit_node->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->branch_next_node_id = exit_node_id;
		new_branch_node->branch_next_node = exit_node;
	} else {
		new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->branch_next_node_id = new_nodes[0]->id;
		new_branch_node->branch_next_node = new_nodes[0];
	}

	this->node_context->next_node_id = new_branch_node->id;
	this->node_context->next_node = new_branch_node;

	new_branch_node->ancestor_ids.push_back(this->node_context->id);

	new_branch_node->network = this->new_network;
	this->new_network = NULL;

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
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

		next_node->ancestor_ids.push_back(new_nodes[n_index]->id);
	}

	wrapper->solution->timestamp++;
}
