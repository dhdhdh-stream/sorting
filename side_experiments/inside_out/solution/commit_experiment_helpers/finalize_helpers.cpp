#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void CommitExperiment::clean() {
	this->node_context->experiment = NULL;
}

void CommitExperiment::add(SolutionWrapper* wrapper) {
	cout << "CommitExperiment add" << endl;

	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		this->new_nodes[n_index]->parent = this->scope_context;
		this->scope_context->nodes[this->new_nodes[n_index]->id] = this->new_nodes[n_index];
	}

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

	int start_node_id;
	AbstractNode* start_node;
	if (this->best_step_types.size() == 0) {
		start_node_id = exit_node_id;
		start_node = exit_node;
	} else {
		start_node_id = new_nodes[0]->id;
		start_node = new_nodes[0];
	}

	switch (this->node_context->type) {
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

			action_node->next_node_id = start_node_id;
			action_node->next_node = start_node;

			start_node->ancestor_ids.push_back(action_node->id);
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

			scope_node->next_node_id = start_node_id;
			scope_node->next_node = start_node;

			start_node->ancestor_ids.push_back(scope_node->id);
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

				branch_node->branch_next_node_id = start_node_id;
				branch_node->branch_next_node = start_node;
			} else {
				for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
					if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
						branch_node->original_next_node->ancestor_ids.erase(
							branch_node->original_next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}

				branch_node->original_next_node_id = start_node_id;
				branch_node->original_next_node = start_node;
			}

			start_node->ancestor_ids.push_back(branch_node->id);
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;

			if (obs_node->next_node != NULL) {
				for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
					if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
						obs_node->next_node->ancestor_ids.erase(
							obs_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
			}

			obs_node->next_node_id = start_node_id;
			obs_node->next_node = start_node;

			start_node->ancestor_ids.push_back(obs_node->id);
		}
		break;
	}

	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)this->new_nodes.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			next_node_id = this->new_nodes[n_index+1]->id;
			next_node = this->new_nodes[n_index+1];
		}

		switch (this->new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->new_nodes[n_index];
				action_node->next_node_id = next_node_id;
				action_node->next_node = next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->new_nodes[n_index];
				scope_node->next_node_id = next_node_id;
				scope_node->next_node = next_node;
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->new_nodes[n_index];
				obs_node->next_node_id = next_node_id;
				obs_node->next_node = next_node;
			}
			break;
		}

		next_node->ancestor_ids.push_back(this->new_nodes[n_index]->id);
	}

	for (int n_index = 0; n_index < (int)this->save_new_nodes.size(); n_index++) {
		this->save_new_nodes[n_index]->parent = this->scope_context;
		this->scope_context->nodes[this->save_new_nodes[n_index]->id] = this->save_new_nodes[n_index];
	}

	int save_exit_node_id;
	AbstractNode* save_exit_node;
	if (this->save_exit_next_node == NULL) {
		if (new_ending_node == NULL) {
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
		}

		save_exit_node_id = new_ending_node->id;
		save_exit_node = new_ending_node;
	} else {
		save_exit_node_id = this->save_exit_next_node->id;
		save_exit_node = this->save_exit_next_node;
	}

	this->scope_context->nodes[this->new_branch_node->id] = this->new_branch_node;

	ObsNode* obs_node = (ObsNode*)this->new_nodes[this->step_iter-1];

	if (this->commit_new_network != NULL) {
		Factor* new_factor = new Factor();
		new_factor->inputs = this->commit_new_network_inputs;
		new_factor->network = this->commit_new_network;
		this->commit_new_network = NULL;

		obs_node->factors.push_back(new_factor);

		Input new_input;
		new_input.scope_context = {this->scope_context};
		new_input.node_context = {obs_node->id};
		new_input.factor_index = (int)obs_node->factors.size()-1;
		new_input.obs_index = -1;
		this->commit_new_inputs.push_back(new_input);
		this->commit_new_input_averages.push_back(0.0);
		this->commit_new_input_standard_deviations.push_back(1.0);
		this->commit_new_weights.push_back(1.0);
	}

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

	if (this->save_step_types.size() == 0) {
		save_exit_node->ancestor_ids.push_back(this->new_branch_node->id);

		this->new_branch_node->branch_next_node_id = save_exit_node_id;
		this->new_branch_node->branch_next_node = save_exit_node;
	} else {
		this->save_new_nodes[0]->ancestor_ids.push_back(this->new_branch_node->id);

		this->new_branch_node->branch_next_node_id = this->save_new_nodes[0]->id;
		this->new_branch_node->branch_next_node = this->save_new_nodes[0];
	}

	obs_node->next_node_id = this->new_branch_node->id;
	obs_node->next_node = this->new_branch_node;

	this->new_branch_node->ancestor_ids.push_back(this->new_nodes[this->step_iter-1]->id);

	this->new_branch_node->average_val = this->commit_new_average_score;
	this->new_branch_node->inputs = this->commit_new_inputs;
	this->new_branch_node->input_averages = this->commit_new_input_averages;
	this->new_branch_node->input_standard_deviations = this->commit_new_input_standard_deviations;
	this->new_branch_node->weights = this->commit_new_weights;

	#if defined(MDEBUG) && MDEBUG
	if (this->verify_problems.size() > 0) {
		wrapper->solution->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		wrapper->solution->verify_seeds = this->verify_seeds;

		this->new_branch_node->verify_key = this;
		this->new_branch_node->verify_scores = this->verify_scores;
	}
	#endif /* MDEBUG */

	this->new_branch_node = NULL;

	for (int n_index = 0; n_index < (int)this->save_new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)this->save_new_nodes.size()-1) {
			next_node_id = save_exit_node_id;
			next_node = save_exit_node;
		} else {
			next_node_id = this->save_new_nodes[n_index+1]->id;
			next_node = this->save_new_nodes[n_index+1];
		}

		switch (this->save_new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->save_new_nodes[n_index];
				action_node->next_node_id = next_node_id;
				action_node->next_node = next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->save_new_nodes[n_index];
				scope_node->next_node_id = next_node_id;
				scope_node->next_node = next_node;
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->save_new_nodes[n_index];
				obs_node->next_node_id = next_node_id;
				obs_node->next_node = next_node;
			}
			break;
		}

		next_node->ancestor_ids.push_back(this->save_new_nodes[n_index]->id);
	}

	this->new_nodes.clear();
	this->save_new_nodes.clear();
}
