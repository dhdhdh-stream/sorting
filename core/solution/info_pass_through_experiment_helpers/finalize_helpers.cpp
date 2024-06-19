#include "info_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void InfoPassThroughExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		new_pass_through(duplicate);

		InfoScope* duplicate_info_scope = duplicate->info_scopes[this->scope_context->id];

		duplicate_info_scope->input_node_contexts.clear();
		for (int n_index = 0; n_index < (int)this->new_input_node_contexts.size(); n_index++) {
			duplicate_info_scope->input_node_contexts.push_back(
				duplicate_info_scope->nodes[this->new_input_node_contexts[n_index]->id]);
		}
		duplicate_info_scope->input_obs_indexes = this->new_input_obs_indexes;
		delete duplicate_info_scope->network;
		duplicate_info_scope->network = this->new_network;
		this->new_network = NULL;

		#if defined(MDEBUG) && MDEBUG
		duplicate->verify_problems = this->verify_problems;
		this->verify_problems.clear();
		duplicate->verify_seeds = this->verify_seeds;

		duplicate_info_scope->verify_key = this;
		duplicate_info_scope->verify_scores = this->verify_scores;
		#endif /* MDEBUG */
	}

	InfoScope* parent_scope = (InfoScope*)this->scope_context;
	parent_scope->experiment = NULL;

	int experiment_index;
	for (int e_index = 0; e_index < (int)this->node_context->experiments.size(); e_index++) {
		if (this->node_context->experiments[e_index] == this) {
			experiment_index = e_index;
			break;
		}
	}
	this->node_context->experiments.erase(this->node_context->experiments.begin() + experiment_index);
}

void InfoPassThroughExperiment::new_pass_through(Solution* duplicate) {
	InfoScope* duplicate_local_scope = duplicate->info_scopes[this->scope_context->id];
	AbstractNode* duplicate_explore_node = duplicate_local_scope->nodes[this->node_context->id];

	int start_node_id;
	AbstractNode* start_node;
	if (this->actions.size() == 0) {
		start_node_id = this->exit_next_node->id;
		start_node = duplicate_local_scope->nodes[this->exit_next_node->id];
	} else {
		start_node_id = this->actions[0]->id;
		start_node = this->actions[0];
	}

	ActionNode* action_node = (ActionNode*)duplicate_explore_node;
	action_node->next_node_id = start_node_id;
	action_node->next_node = start_node;

	if (this->ending_node != NULL) {
		this->ending_node->parent = duplicate_local_scope;
		duplicate_local_scope->nodes[this->ending_node->id] = this->ending_node;
	}

	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		this->actions[s_index]->parent = duplicate_local_scope;
		duplicate_local_scope->nodes[this->actions[s_index]->id] = this->actions[s_index];
	}
	if (this->actions.size() > 0) {
		if (this->actions.back()->next_node != NULL) {
			this->actions.back()->next_node = duplicate_local_scope
				->nodes[this->actions.back()->next_node->id];
		}
	}

	this->actions.clear();
	this->ending_node = NULL;
}
