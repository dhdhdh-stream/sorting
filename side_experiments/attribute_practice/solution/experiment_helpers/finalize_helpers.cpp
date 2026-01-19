#include "experiment.h"

#include <iostream>
#include <sstream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void Experiment::clean() {
	this->node_context->experiment = NULL;
}

void Experiment::add(SolutionWrapper* wrapper) {
	stringstream ss;
	ss << "Experiment" << "; ";
	ss << "this->scope_context->id: " << this->scope_context->id << "; ";
	ss << "this->node_context->id: " << this->node_context->id << "; ";
	ss << "this->is_branch: " << this->is_branch << "; ";
	ss << "new explore path:";
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ss << " " << this->best_actions[s_index];
		} else {
			ss << " E" << this->best_scopes[s_index]->id;
		}
	}
	ss << "; ";

	if (this->best_exit_next_node == NULL) {
		ss << "this->best_exit_next_node->id: " << -1 << "; ";
	} else {
		ss << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << "; ";
	}

	ss << "this->remove_impact: " << this->remove_impact << "; ";

	ss << "this->is_binarize: " << this->is_binarize << "; ";

	ss << "this->improvement: " << this->improvement << "; ";

	wrapper->solution->improvement_history.push_back(calc_new_score());
	wrapper->solution->change_history.push_back(ss.str());

	cout << ss.str() << endl;

	if (this->best_new_scope != NULL) {
		wrapper->solution->scopes.push_back(this->best_new_scope);
		this->best_new_scope->id = (int)wrapper->solution->scopes.size()-1;

		recursive_add_child(scope_context,
							wrapper,
							this->best_new_scope);

		this->best_new_scope = NULL;
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = (ActionNode*)this->best_new_nodes[s_index];
			new_action_node->id = scope_context->node_counter + s_index;
			scope_context->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = this->best_actions[s_index];
		} else {
			scope_context->nodes[this->best_new_nodes[s_index]->id] = this->best_new_nodes[s_index];
		}
	}
	scope_context->nodes[this->new_branch_node->id] = this->new_branch_node;
	scope_context->node_counter += (int)this->best_step_types.size() + 1;

	ObsNode* new_ending_node = NULL;

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->best_exit_next_node == NULL) {
		new_ending_node = new ObsNode();
		new_ending_node->parent = this->scope_context;
		new_ending_node->id = this->scope_context->node_counter;
		this->scope_context->node_counter++;

		for (map<int, AbstractNode*>::iterator it = this->scope_context->nodes.begin();
				it != this->scope_context->nodes.end(); it++) {
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

		this->scope_context->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->best_exit_next_node->id;
		exit_node = this->best_exit_next_node;
	}

	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;

			for (int a_index = 0; a_index < (int)start_node->next_node->ancestor_ids.size(); a_index++) {
				if (start_node->next_node->ancestor_ids[a_index] == start_node->id) {
					start_node->next_node->ancestor_ids.erase(
						start_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}
			start_node->next_node->ancestor_ids.push_back(this->new_branch_node->id);

			this->new_branch_node->original_next_node_id = start_node->next_node_id;
			this->new_branch_node->original_next_node = start_node->next_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
				if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
					action_node->next_node->ancestor_ids.erase(
						action_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}
			action_node->next_node->ancestor_ids.push_back(this->new_branch_node->id);

			this->new_branch_node->original_next_node_id = action_node->next_node_id;
			this->new_branch_node->original_next_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
				if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
					scope_node->next_node->ancestor_ids.erase(
						scope_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}
			scope_node->next_node->ancestor_ids.push_back(this->new_branch_node->id);

			this->new_branch_node->original_next_node_id = scope_node->next_node_id;
			this->new_branch_node->original_next_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
					if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
						branch_node->branch_next_node->ancestor_ids.erase(
							branch_node->branch_next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				branch_node->branch_next_node->ancestor_ids.push_back(this->new_branch_node->id);

				this->new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
				this->new_branch_node->original_next_node = branch_node->branch_next_node;
			} else {
				for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
					if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
						branch_node->original_next_node->ancestor_ids.erase(
							branch_node->original_next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				branch_node->original_next_node->ancestor_ids.push_back(this->new_branch_node->id);

				this->new_branch_node->original_next_node_id = branch_node->original_next_node_id;
				this->new_branch_node->original_next_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;

			if (obs_node->next_node == NULL) {
				if (new_ending_node != NULL) {
					new_ending_node->ancestor_ids.push_back(this->new_branch_node->id);

					this->new_branch_node->original_next_node_id = new_ending_node->id;
					this->new_branch_node->original_next_node = new_ending_node;
				} else {
					new_ending_node = new ObsNode();
					new_ending_node->parent = this->scope_context;
					new_ending_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					for (map<int, AbstractNode*>::iterator it = this->scope_context->nodes.begin();
							it != this->scope_context->nodes.end(); it++) {
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

					this->scope_context->nodes[new_ending_node->id] = new_ending_node;

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;

					new_ending_node->ancestor_ids.push_back(this->new_branch_node->id);

					this->new_branch_node->original_next_node_id = new_ending_node->id;
					this->new_branch_node->original_next_node = new_ending_node;
				}
			} else {
				for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
					if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
						obs_node->next_node->ancestor_ids.erase(
							obs_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				obs_node->next_node->ancestor_ids.push_back(this->new_branch_node->id);

				this->new_branch_node->original_next_node_id = obs_node->next_node_id;
				this->new_branch_node->original_next_node = obs_node->next_node;
			}
		}
		break;
	}

	if (this->best_step_types.size() == 0) {
		exit_node->ancestor_ids.push_back(this->new_branch_node->id);

		this->new_branch_node->branch_next_node_id = exit_node_id;
		this->new_branch_node->branch_next_node = exit_node;
	} else {
		this->best_new_nodes[0]->ancestor_ids.push_back(this->new_branch_node->id);

		this->new_branch_node->branch_next_node_id = this->best_new_nodes[0]->id;
		this->new_branch_node->branch_next_node = this->best_new_nodes[0];
	}

	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;

			start_node->next_node_id = this->new_branch_node->id;
			start_node->next_node = this->new_branch_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			action_node->next_node_id = this->new_branch_node->id;
			action_node->next_node = this->new_branch_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			scope_node->next_node_id = this->new_branch_node->id;
			scope_node->next_node = this->new_branch_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;

			if (this->is_branch) {
				branch_node->branch_next_node_id = this->new_branch_node->id;
				branch_node->branch_next_node = this->new_branch_node;
			} else {
				branch_node->original_next_node_id = this->new_branch_node->id;
				branch_node->original_next_node = this->new_branch_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;

			obs_node->next_node_id = this->new_branch_node->id;
			obs_node->next_node = this->new_branch_node;
		}
		break;
	}
	this->new_branch_node->ancestor_ids.push_back(this->node_context->id);

	this->new_branch_node->network = this->new_true_network;
	this->new_true_network = NULL;

	#if defined(MDEBUG) && MDEBUG
	if (this->verify_problems.size() > 0) {
		wrapper->solution->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		wrapper->solution->verify_seeds = this->verify_seeds;

		this->new_branch_node->verify_key = this;
		this->new_branch_node->verify_scores = this->verify_scores;
	}
	#endif /* MDEBUG */

	for (int n_index = 0; n_index < (int)this->best_new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)this->best_new_nodes.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			next_node_id = this->best_new_nodes[n_index+1]->id;
			next_node = this->best_new_nodes[n_index+1];
		}

		switch (this->best_new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->best_new_nodes[n_index];
				action_node->next_node_id = next_node_id;
				action_node->next_node = next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->best_new_nodes[n_index];
				scope_node->next_node_id = next_node_id;
				scope_node->next_node = next_node;
			}
			break;
		}

		next_node->ancestor_ids.push_back(this->best_new_nodes[n_index]->id);
	}

	this->best_new_nodes.clear();
	this->new_branch_node = NULL;
}

double Experiment::calc_new_score() {
	return this->total_sum_scores / this->total_count;
}
