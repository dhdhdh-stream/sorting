#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void BranchExperiment::clean() {
	this->node_context->experiment = NULL;
}

void recursive_add_child(Scope* curr_parent,
						 SolutionWrapper* wrapper,
						 Scope* new_scope) {
	curr_parent->child_scopes.push_back(new_scope);

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		bool is_needed = false;
		bool is_added = false;
		for (int c_index = 0; c_index < (int)wrapper->solution->scopes[s_index]->child_scopes.size(); c_index++) {
			if (wrapper->solution->scopes[s_index]->child_scopes[c_index] == curr_parent) {
				is_needed = true;
			}

			if (wrapper->solution->scopes[s_index]->child_scopes[c_index] == new_scope) {
				is_added = true;
			}
		}

		if (is_needed && !is_added) {
			recursive_add_child(wrapper->solution->scopes[s_index],
								wrapper,
								new_scope);
		}
	}
}

void BranchExperiment::add(SolutionWrapper* wrapper) {
	cout << "BranchExperiment add" << endl;
	cout << "this->scope_context->id: " << this->scope_context->id << endl;
	cout << "this->node_context->id: " << this->node_context->id << endl;
	cout << "this->is_branch: " << this->is_branch << endl;
	cout << "new explore path:";
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			cout << " " << this->best_actions[s_index];
		} else {
			cout << " E" << this->best_scopes[s_index]->id;
		}
	}
	cout << endl;

	if (this->best_exit_next_node == NULL) {
		cout << "this->best_exit_next_node->id: " << -1 << endl;
	} else {
		cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
	}

	cout << "this->select_percentage: " << this->select_percentage << endl;

	if (this->best_new_scope != NULL) {
		wrapper->solution->scopes.push_back(this->best_new_scope);
		this->best_new_scope->id = (int)wrapper->solution->scopes.size()-1;
		this->best_new_scope->is_external = false;

		clean_scope(this->best_new_scope,
					wrapper);

		recursive_add_child(scope_context,
							wrapper,
							this->best_new_scope);

		this->best_new_scope = NULL;
	}

	vector<AbstractNode*> new_nodes;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = scope_context;
			new_action_node->id = scope_context->node_counter;
			scope_context->node_counter++;
			scope_context->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = this->best_actions[s_index];

			new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = scope_context;
			new_scope_node->id = scope_context->node_counter;
			scope_context->node_counter++;
			scope_context->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->scope = this->best_scopes[s_index];

			new_nodes.push_back(new_scope_node);
		}
	}

	/**
	 * - simply always create new ending node
	 */
	ObsNode* new_ending_node = new ObsNode();
	new_ending_node->parent = this->scope_context;
	new_ending_node->id = this->scope_context->node_counter;
	this->scope_context->node_counter++;

	if (this->best_exit_next_node == NULL) {
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

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;
	} else if (this->best_exit_next_node->ancestor_ids.size() > 1) {
		ObsNode* path_end = (ObsNode*)fetch_path_end(this->node_context);

		for (int a_index = 0; a_index < (int)this->best_exit_next_node->ancestor_ids.size(); a_index++) {
			if (this->best_exit_next_node->ancestor_ids[a_index] == path_end->id) {
				this->best_exit_next_node->ancestor_ids.erase(
					this->best_exit_next_node->ancestor_ids.begin() + a_index);
				break;
			}
		}
		this->best_exit_next_node->ancestor_ids.push_back(new_ending_node->id);

		path_end->next_node_id = new_ending_node->id;
		path_end->next_node = new_ending_node;

		new_ending_node->ancestor_ids.push_back(path_end->id);

		new_ending_node->next_node_id = this->best_exit_next_node->id;
		new_ending_node->next_node = this->best_exit_next_node;
	} else {
		AbstractNode* previous_node = this->scope_context->nodes[this->best_exit_next_node->ancestor_ids[0]];

		this->best_exit_next_node->ancestor_ids.clear();
		this->best_exit_next_node->ancestor_ids.push_back(new_ending_node->id);

		switch (previous_node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)previous_node;
				start_node->next_node_id = new_ending_node->id;
				start_node->next_node = new_ending_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)previous_node;
				action_node->next_node_id = new_ending_node->id;
				action_node->next_node = new_ending_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)previous_node;
				scope_node->next_node_id = new_ending_node->id;
				scope_node->next_node = new_ending_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)previous_node;
				/**
				 * - must be this->node_context
				 */
				if (this->is_branch) {
					branch_node->branch_next_node_id = new_ending_node->id;
					branch_node->branch_next_node = new_ending_node;
				} else {
					branch_node->original_next_node_id = new_ending_node->id;
					branch_node->original_next_node = new_ending_node;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)previous_node;
				obs_node->next_node_id = new_ending_node->id;
				obs_node->next_node = new_ending_node;
			}
			break;
		}

		new_ending_node->ancestor_ids.push_back(previous_node->id);

		new_ending_node->next_node_id = this->best_exit_next_node->id;
		new_ending_node->next_node = this->best_exit_next_node;
	}

	this->scope_context->nodes[new_ending_node->id] = new_ending_node;

	if (this->select_percentage == 1.0) {
		int starting_node_id;
		AbstractNode* starting_node;
		if (this->best_step_types.size() == 0) {
			starting_node_id = new_ending_node->id;
			starting_node = new_ending_node;
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
		new_branch_node->parent = scope_context;
		new_branch_node->id = scope_context->node_counter;
		scope_context->node_counter++;
		scope_context->nodes[new_branch_node->id] = new_branch_node;

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

		if (this->best_step_types.size() == 0) {
			new_ending_node->ancestor_ids.push_back(new_branch_node->id);

			new_branch_node->branch_next_node_id = new_ending_node->id;
			new_branch_node->branch_next_node = new_ending_node;
		} else {
			new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

			new_branch_node->branch_next_node_id = new_nodes[0]->id;
			new_branch_node->branch_next_node = new_nodes[0];
		}

		new_branch_node->branch_end_node_id = new_ending_node->id;
		new_branch_node->branch_end_node = new_ending_node;

		new_ending_node->branch_start_node_id = new_branch_node->id;
		new_ending_node->branch_start_node = new_branch_node;

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

		new_branch_node->constant = this->new_constant;
		new_branch_node->inputs = this->new_inputs;
		new_branch_node->input_averages = this->new_input_averages;
		new_branch_node->input_standard_deviations = this->new_input_standard_deviations;
		new_branch_node->weights = this->new_weights;

		#if defined(MDEBUG) && MDEBUG
		if (this->verify_problems.size() > 0) {
			wrapper->solution->verify_problems = this->verify_problems;
			this->verify_problems.clear();
			wrapper->solution->verify_seeds = this->verify_seeds;

			new_branch_node->verify_key = this;
			new_branch_node->verify_scores = this->verify_scores;
		}
		#endif /* MDEBUG */
	}

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			next_node_id = new_ending_node->id;
			next_node = new_ending_node;
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
}
