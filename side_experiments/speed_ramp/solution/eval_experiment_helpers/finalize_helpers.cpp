#include "eval_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "signal_experiment.h"
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
	if (this->new_scope != NULL) {
		this->new_scope->id = wrapper->scope_counter;
		wrapper->scope_counter++;
		wrapper->solution->scopes[this->new_scope->id] = this->new_scope;

		clean_scope(this->new_scope,
					wrapper);

		this->new_scope->child_scopes = this->scope_context->child_scopes;
		recursive_add_child(this->scope_context,
							wrapper,
							this->new_scope);

		this->new_scope = NULL;
	}

	vector<AbstractNode*> new_nodes;
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = this->scope_context;
			new_action_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;
			this->scope_context->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = this->actions[s_index];

			new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = this->scope_context;
			new_scope_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;
			this->scope_context->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->scope = this->scopes[s_index];

			new_nodes.push_back(new_scope_node);

			this->scopes[s_index]->num_generalize_successes++;
			if (this->scopes[s_index]->num_generalize_successes >= 3
					&& this->scopes[s_index]->signal_experiment == NULL) {
				this->scopes[s_index]->signal_experiment = new SignalExperiment(this->scopes[s_index]);
			}
		}
	}

	ObsNode* new_ending_node = NULL;

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
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
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	if (this->select_percentage == 1.0) {
		int starting_node_id;
		AbstractNode* starting_node;
		if (this->step_types.size() == 0) {
			starting_node_id = exit_node_id;
			starting_node = exit_node;
		} else {
			starting_node_id = new_nodes[0]->id;
			starting_node = new_nodes[0];
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

				start_node->next_node_id = starting_node_id;
				start_node->next_node = starting_node;

				starting_node->ancestor_ids.push_back(start_node->id);
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

				action_node->next_node_id = starting_node_id;
				action_node->next_node = starting_node;

				starting_node->ancestor_ids.push_back(action_node->id);
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

				scope_node->next_node_id = starting_node_id;
				scope_node->next_node = starting_node;

				starting_node->ancestor_ids.push_back(scope_node->id);
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

					branch_node->branch_next_node_id = starting_node_id;
					branch_node->branch_next_node = starting_node;
				} else {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}

					branch_node->original_next_node_id = starting_node_id;
					branch_node->original_next_node = starting_node;
				}

				starting_node->ancestor_ids.push_back(branch_node->id);
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

				obs_node->next_node_id = starting_node_id;
				obs_node->next_node = starting_node;

				starting_node->ancestor_ids.push_back(obs_node->id);
			}
			break;
		}
	} else {
		BranchNode* new_branch_node = new BranchNode();
		new_branch_node->parent = this->scope_context;
		new_branch_node->id = this->scope_context->node_counter;
		this->scope_context->node_counter++;
		this->scope_context->nodes[new_branch_node->id] = new_branch_node;

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
				start_node->next_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->original_next_node_id = start_node->next_node_id;
				new_branch_node->original_next_node = start_node->next_node;
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
				action_node->next_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->original_next_node_id = action_node->next_node_id;
				new_branch_node->original_next_node = action_node->next_node;
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
				scope_node->next_node->ancestor_ids.push_back(new_branch_node->id);

				new_branch_node->original_next_node_id = scope_node->next_node_id;
				new_branch_node->original_next_node = scope_node->next_node;
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
					branch_node->branch_next_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
					new_branch_node->original_next_node = branch_node->branch_next_node;
				} else {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->original_next_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = branch_node->original_next_node_id;
					new_branch_node->original_next_node = branch_node->original_next_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;

				if (obs_node->next_node == NULL) {
					if (new_ending_node != NULL) {
						new_ending_node->ancestor_ids.push_back(new_branch_node->id);

						new_branch_node->original_next_node_id = new_ending_node->id;
						new_branch_node->original_next_node = new_ending_node;
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

						new_ending_node->ancestor_ids.push_back(new_branch_node->id);

						new_branch_node->original_next_node_id = new_ending_node->id;
						new_branch_node->original_next_node = new_ending_node;
					}
				} else {
					for (int a_index = 0; a_index < (int)obs_node->next_node->ancestor_ids.size(); a_index++) {
						if (obs_node->next_node->ancestor_ids[a_index] == obs_node->id) {
							obs_node->next_node->ancestor_ids.erase(
								obs_node->next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					obs_node->next_node->ancestor_ids.push_back(new_branch_node->id);

					new_branch_node->original_next_node_id = obs_node->next_node_id;
					new_branch_node->original_next_node = obs_node->next_node;
				}
			}
			break;
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

		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;

				start_node->next_node_id = new_branch_node->id;
				start_node->next_node = new_branch_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;

				action_node->next_node_id = new_branch_node->id;
				action_node->next_node = new_branch_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;

				scope_node->next_node_id = new_branch_node->id;
				scope_node->next_node = new_branch_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;

				if (this->is_branch) {
					branch_node->branch_next_node_id = new_branch_node->id;
					branch_node->branch_next_node = new_branch_node;
				} else {
					branch_node->original_next_node_id = new_branch_node->id;
					branch_node->original_next_node = new_branch_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;

				obs_node->next_node_id = new_branch_node->id;
				obs_node->next_node = new_branch_node;
			}
			break;
		}
		new_branch_node->ancestor_ids.push_back(this->node_context->id);

		if (this->new_network != NULL) {
			Factor* new_factor = new Factor();
			new_factor->inputs = this->new_network_inputs;
			new_factor->network = this->new_network;
			this->new_network = NULL;
			new_factor->is_meaningful = true;

			this->scope_context->factors.push_back(new_factor);

			new_factor->link((int)this->scope_context->factors.size()-1);

			Input new_input;
			new_input.scope_context = {this->scope_context};
			new_input.factor_index = (int)this->scope_context->factors.size()-1;
			new_input.node_context = {-1};
			new_input.obs_index = -1;
			this->new_inputs.push_back(new_input);
			this->new_input_averages.push_back(0.0);
			this->new_input_standard_deviations.push_back(1.0);
			this->new_weights.push_back(1.0);
		}

		new_branch_node->average_val = this->new_average_score;
		new_branch_node->inputs = this->new_inputs;
		new_branch_node->input_averages = this->new_input_averages;
		new_branch_node->input_standard_deviations = this->new_input_standard_deviations;
		new_branch_node->weights = this->new_weights;
	}

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
