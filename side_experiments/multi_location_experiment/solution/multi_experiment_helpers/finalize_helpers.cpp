#include "multi_experiment.h"

#include <ctime>
#include <iostream>
#include <sstream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void MultiExperiment::add(SolutionWrapper* wrapper) {
	if (wrapper->prev_solution != NULL) {
		delete wrapper->prev_solution;
	}
	wrapper->prev_solution = new Solution(wrapper->solution);

	stringstream ss;
	ss << get_time() << "; ";
	ss << "timestamp: " << wrapper->solution->timestamp << "; ";
	ss << "curr_num_resets: " << wrapper->solution->curr_num_resets << "; ";
	ss << "MultiExperiment" << "; ";
	ss << "this->scope_context->id: " << this->scope_context->id << "; ";
	for (int c_index = 0; c_index < (int)this->node_contexts.size(); c_index++) {
		ss << "this->node_context->id: " << this->node_contexts[c_index]->id << "; ";
		ss << "this->is_branch: " << this->is_branch[c_index] << "; ";
	}
	ss << "new explore path:";
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ss << " " << this->best_actions[s_index];
		} else {
			ss << " E" << this->best_scopes[s_index]->id;
		}
	}
	ss << "; ";

	wrapper->solution->improvement_history.push_back(wrapper->solution->curr_score);
	cout << "previous_val_average: " << wrapper->solution->curr_score << endl;

	wrapper->solution->change_history.push_back(ss.str());

	cout << ss.str() << endl;

	Scope* new_scope = new Scope();
	new_scope->id = wrapper->solution->scopes.size();
	new_scope->node_counter = 0;
	wrapper->solution->scopes.push_back(new_scope);

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (this->scope_context == scope->child_scopes[c_index]) {
				scope->child_scopes.push_back(new_scope);
				break;
			}
		}
	}
	this->scope_context->child_scopes.push_back(new_scope);

	NoopNode* start_node = new NoopNode();
	start_node->parent = new_scope;
	start_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[start_node->id] = start_node;

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = new_scope;
	new_branch_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[new_branch_node->id] = new_branch_node;

	new_branch_node->original_network = this->existing_network;
	this->existing_network = NULL;
	new_branch_node->branch_network = this->new_network;
	this->new_network = NULL;

	new_branch_node->consec_original = 0;
	new_branch_node->consec_branch = 0;

	vector<AbstractNode*> new_nodes;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = new_scope;
			new_action_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = this->best_actions[s_index];

			new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = new_scope;
			new_scope_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->scope = this->best_scopes[s_index];

			new_nodes.push_back(new_scope_node);
		}
	}

	NoopNode* end_node = new NoopNode();
	end_node->parent = new_scope;
	end_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[end_node->id] = end_node;

	start_node->next_node_id = new_branch_node->id;
	start_node->next_node = new_branch_node;

	new_branch_node->ancestor_ids.push_back(start_node->id);

	new_branch_node->original_next_node_id = end_node->id;
	new_branch_node->original_next_node = end_node;

	end_node->ancestor_ids.push_back(new_branch_node->id);

	new_branch_node->branch_next_node_id = new_nodes[0]->id;
	new_branch_node->branch_next_node = new_nodes[0];

	new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			next_node_id = end_node->id;
			next_node = end_node;
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

	end_node->next_node_id = -1;
	end_node->next_node = NULL;

	for (int c_index = 0; c_index < (int)this->node_contexts.size(); c_index++) {
		ScopeNode* new_scope_node = new ScopeNode();
		new_scope_node->parent = this->scope_context;
		new_scope_node->id = this->scope_context->node_counter;
		this->scope_context->node_counter++;
		this->scope_context->nodes[new_scope_node->id] = new_scope_node;

		new_scope_node->scope = new_scope;

		switch (this->node_contexts[c_index]->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_contexts[c_index];
				if (noop_node->next_node == NULL) {
					NoopNode* new_ending_node = new NoopNode();
					new_ending_node->parent = this->scope_context;
					new_ending_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					for (map<int, AbstractNode*>::iterator it = this->scope_context->nodes.begin();
							it != this->scope_context->nodes.end(); it++) {
						if (it->second->type == NODE_TYPE_NOOP) {
							NoopNode* p_noop_node = (NoopNode*)it->second;
							if (p_noop_node->next_node == NULL) {
								p_noop_node->next_node_id = new_ending_node->id;
								p_noop_node->next_node = new_ending_node;

								new_ending_node->ancestor_ids.push_back(p_noop_node->id);

								break;
							}
						}
					}

					this->scope_context->nodes[new_ending_node->id] = new_ending_node;

					new_ending_node->next_node_id = -1;
					new_ending_node->next_node = NULL;

					new_ending_node->ancestor_ids.push_back(new_scope_node->id);

					new_scope_node->next_node_id = new_ending_node->id;
					new_scope_node->next_node = new_ending_node;
				} else {
					for (int a_index = 0; a_index < (int)noop_node->next_node->ancestor_ids.size(); a_index++) {
						if (noop_node->next_node->ancestor_ids[a_index] == noop_node->id) {
							noop_node->next_node->ancestor_ids.erase(
								noop_node->next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					noop_node->next_node->ancestor_ids.push_back(new_scope_node->id);

					new_scope_node->next_node_id = noop_node->next_node_id;
					new_scope_node->next_node = noop_node->next_node;
				}

				noop_node->next_node_id = new_scope_node->id;
				noop_node->next_node = new_scope_node;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_contexts[c_index];

				for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
					if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
						action_node->next_node->ancestor_ids.erase(
							action_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				action_node->next_node->ancestor_ids.push_back(new_scope_node->id);

				new_scope_node->next_node_id = action_node->next_node_id;
				new_scope_node->next_node = action_node->next_node;

				action_node->next_node_id = new_scope_node->id;
				action_node->next_node = new_scope_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_contexts[c_index];

				for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
					if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
						scope_node->next_node->ancestor_ids.erase(
							scope_node->next_node->ancestor_ids.begin() + a_index);
						break;
					}
				}
				scope_node->next_node->ancestor_ids.push_back(new_scope_node->id);

				new_scope_node->next_node_id = scope_node->next_node_id;
				new_scope_node->next_node = scope_node->next_node;

				scope_node->next_node_id = new_scope_node->id;
				scope_node->next_node = new_scope_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_contexts[c_index];

				if (this->is_branch[c_index]) {
					for (int a_index = 0; a_index < (int)branch_node->branch_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->branch_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->branch_next_node->ancestor_ids.erase(
								branch_node->branch_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->branch_next_node->ancestor_ids.push_back(new_scope_node->id);

					new_scope_node->next_node_id = branch_node->branch_next_node_id;
					new_scope_node->next_node = branch_node->branch_next_node;

					branch_node->branch_next_node_id = new_scope_node->id;
					branch_node->branch_next_node = new_scope_node;
				} else {
					for (int a_index = 0; a_index < (int)branch_node->original_next_node->ancestor_ids.size(); a_index++) {
						if (branch_node->original_next_node->ancestor_ids[a_index] == branch_node->id) {
							branch_node->original_next_node->ancestor_ids.erase(
								branch_node->original_next_node->ancestor_ids.begin() + a_index);
							break;
						}
					}
					branch_node->original_next_node->ancestor_ids.push_back(new_scope_node->id);

					new_scope_node->next_node_id = branch_node->original_next_node_id;
					new_scope_node->next_node = branch_node->original_next_node;

					branch_node->original_next_node_id = new_scope_node->id;
					branch_node->original_next_node = new_scope_node;
				}
			}
			break;
		}
	}

	wrapper->solution->timestamp++;
	wrapper->solution->curr_num_resets = 0;

	if (this->scope_context == wrapper->solution->starting_scope) {
		wrapper->solution->starting_num_improvements++;
		if (wrapper->solution->starting_num_improvements >= GENERALIZE_ITER) {
			Scope* new_scope = new Scope();
			new_scope->id = wrapper->solution->scopes.size();
			new_scope->node_counter = 0;
			wrapper->solution->scopes.push_back(new_scope);

			new_scope->child_scopes = wrapper->solution->starting_scope->child_scopes;
			new_scope->child_scopes.push_back(wrapper->solution->starting_scope);

			new_scope->last_scores = wrapper->solution->starting_scope->last_scores;

			NoopNode* start_node = new NoopNode();
			start_node->parent = new_scope;
			start_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[start_node->id] = start_node;

			ScopeNode* scope_node = new ScopeNode();
			scope_node->parent = new_scope;
			scope_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[scope_node->id] = scope_node;

			scope_node->scope = wrapper->solution->starting_scope;

			NoopNode* end_node = new NoopNode();
			end_node->parent = new_scope;
			end_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[end_node->id] = end_node;

			start_node->next_node_id = scope_node->id;
			start_node->next_node = scope_node;

			scope_node->ancestor_ids.push_back(start_node->id);

			scope_node->next_node_id = end_node->id;
			scope_node->next_node = end_node;

			end_node->ancestor_ids.push_back(scope_node->id);

			end_node->next_node_id = -1;
			end_node->next_node = NULL;

			wrapper->solution->starting_scope = new_scope;
			wrapper->solution->starting_num_improvements = 0;
		}
	}

	wrapper->experiment_iter = EXPERIMENT_REFRESH_NUM_ITERS;
	/**
	 * - reset all other experiments
	 */

	wrapper->iters_since_update = 0;

	// temp
	{
		double val_average = measure_helper(wrapper);
		cout << "post val_average: " << val_average << endl;
	}
}
