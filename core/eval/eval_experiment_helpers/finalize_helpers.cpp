#include "eval_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "eval.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void EvalExperiment::finalize(Solution* duplicate) {
	if (this->result == EXPERIMENT_RESULT_SUCCESS) {
		if (this->is_pass_through) {
			new_pass_through(duplicate);
		} else {
			new_branch(duplicate);
		}

		duplicate->eval->average_score = this->original_average_score;

		duplicate->eval->input_node_context_ids.clear();
		duplicate->eval->input_node_contexts.clear();
		duplicate->eval->input_obs_indexes.clear();

		vector<int> input_mapping(this->eval_input_node_contexts.size(), -1);
		for (int i_index = 0; i_index < (int)this->eval_linear_weights.size(); i_index++) {
			if (this->eval_linear_weights[i_index] != 0.0) {
				if (input_mapping[i_index] == -1) {
					input_mapping[i_index] = (int)duplicate->eval->input_node_contexts.size();
					duplicate->eval->input_node_context_ids.push_back(
						vector<int>{this->eval_input_node_contexts[i_index][0]->id});
					duplicate->eval->input_node_contexts.push_back(
						vector<AbstractNode*>{duplicate->eval->subscope->nodes[
							this->eval_input_node_contexts[i_index][0]->id]});
					duplicate->eval->input_obs_indexes.push_back(this->eval_input_obs_indexes[i_index]);
				}
			}
		}
		for (int i_index = 0; i_index < (int)this->eval_network_input_indexes.size(); i_index++) {
			for (int v_index = 0; v_index < (int)this->eval_network_input_indexes[i_index].size(); v_index++) {
				int original_index = this->eval_network_input_indexes[i_index][v_index];
				if (input_mapping[original_index] == -1) {
					input_mapping[original_index] = (int)duplicate->eval->input_node_contexts.size();
					duplicate->eval->input_node_context_ids.push_back(
						vector<int>{this->eval_input_node_contexts[original_index][0]->id});
					duplicate->eval->input_node_contexts.push_back(
						vector<AbstractNode*>{duplicate->eval->subscope->nodes[
							this->eval_input_node_contexts[original_index][0]->id]});
					duplicate->eval->input_obs_indexes.push_back(this->eval_input_obs_indexes[original_index]);
				}
			}
		}

		duplicate->eval->linear_input_indexes.clear();
		duplicate->eval->linear_weights.clear();
		for (int i_index = 0; i_index < (int)this->eval_linear_weights.size(); i_index++) {
			if (this->eval_linear_weights[i_index] != 0.0) {
				duplicate->eval->linear_input_indexes.push_back(input_mapping[i_index]);
				duplicate->eval->linear_weights.push_back(this->eval_linear_weights[i_index]);
			}
		}

		duplicate->eval->network_input_indexes.clear();
		if (duplicate->eval->network != NULL) {
			delete duplicate->eval->network;
		}
		for (int i_index = 0; i_index < (int)this->eval_network_input_indexes.size(); i_index++) {
			vector<int> input_indexes;
			for (int v_index = 0; v_index < (int)this->eval_network_input_indexes[i_index].size(); v_index++) {
				input_indexes.push_back(input_mapping[this->eval_network_input_indexes[i_index][v_index]]);
			}
			duplicate->eval->network_input_indexes.push_back(input_indexes);
		}
		duplicate->eval->network = this->eval_network;
		this->eval_network = NULL;
	}

	int experiment_index;
	for (int e_index = 0; e_index < (int)this->node_context->experiments.size(); e_index++) {
		if (this->node_context->experiments[e_index] == this) {
			experiment_index = e_index;
		}
	}
	this->node_context->experiments.erase(this->node_context->experiments.begin() + experiment_index);
}

void EvalExperiment::new_branch(Solution* duplicate) {
	cout << "new_branch" << endl;

	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		this->actions[a_index]->parent = duplicate->eval->subscope;
		this->actions[a_index]->id = duplicate->eval->subscope->node_counter;
		duplicate->eval->subscope->node_counter++;
		duplicate->eval->subscope->nodes[this->actions[a_index]->id] = this->actions[a_index];
	}

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
		ActionNode* new_ending_node = new ActionNode();
		new_ending_node->parent = duplicate->eval->subscope;
		new_ending_node->id = duplicate->eval->subscope->node_counter;
		duplicate->eval->subscope->node_counter++;
		duplicate->eval->subscope->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->action = Action(ACTION_NOOP);

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (a_index == (int)this->actions.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			next_node_id = this->actions[a_index+1]->id;
			next_node = this->actions[a_index+1];
		}

		this->actions[a_index]->next_node_id = next_node_id;
		this->actions[a_index]->next_node = next_node;
	}

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = duplicate->eval->subscope;
	new_branch_node->id = duplicate->eval->subscope->node_counter;
	duplicate->eval->subscope->node_counter++;
	duplicate->eval->subscope->nodes[new_branch_node->id] = new_branch_node;

	new_branch_node->original_average_score = this->existing_average_score;
	new_branch_node->branch_average_score = this->new_average_score;

	vector<int> input_mapping(this->decision_input_node_contexts.size(), -1);
	for (int i_index = 0; i_index < (int)this->existing_linear_weights.size(); i_index++) {
		if (this->existing_linear_weights[i_index] != 0.0) {
			if (input_mapping[i_index] == -1) {
				input_mapping[i_index] = (int)new_branch_node->input_scope_contexts.size();
				new_branch_node->input_scope_context_ids.push_back(vector<int>{-1});
				new_branch_node->input_scope_contexts.push_back(
					vector<Scope*>{duplicate->eval->subscope});
				new_branch_node->input_node_context_ids.push_back(
					vector<int>{this->decision_input_node_contexts[i_index][0]->id});
				new_branch_node->input_node_contexts.push_back(
					vector<AbstractNode*>{duplicate->eval->subscope->nodes[
						this->decision_input_node_contexts[i_index][0]->id]});
				new_branch_node->input_obs_indexes.push_back(this->decision_input_obs_indexes[i_index]);
			}
		}
	}
	for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
		for (int v_index = 0; v_index < (int)this->existing_network_input_indexes[i_index].size(); v_index++) {
			int original_index = this->existing_network_input_indexes[i_index][v_index];
			if (input_mapping[original_index] == -1) {
				input_mapping[original_index] = (int)new_branch_node->input_scope_contexts.size();
				new_branch_node->input_scope_context_ids.push_back(vector<int>{-1});
				new_branch_node->input_scope_contexts.push_back(
					vector<Scope*>{duplicate->eval->subscope});
				new_branch_node->input_node_context_ids.push_back(
					vector<int>{this->decision_input_node_contexts[original_index][0]->id});
				new_branch_node->input_node_contexts.push_back(
					vector<AbstractNode*>{duplicate->eval->subscope->nodes[
						this->decision_input_node_contexts[original_index][0]->id]});
				new_branch_node->input_obs_indexes.push_back(this->decision_input_obs_indexes[original_index]);
			}
		}
	}
	for (int i_index = 0; i_index < (int)this->new_linear_weights.size(); i_index++) {
		if (this->new_linear_weights[i_index] != 0.0) {
			if (input_mapping[i_index] == -1) {
				input_mapping[i_index] = (int)new_branch_node->input_scope_contexts.size();
				new_branch_node->input_scope_context_ids.push_back(vector<int>{-1});
				new_branch_node->input_scope_contexts.push_back(
					vector<Scope*>{duplicate->eval->subscope});
				new_branch_node->input_node_context_ids.push_back(
					vector<int>{this->decision_input_node_contexts[i_index][0]->id});
				new_branch_node->input_node_contexts.push_back(
					vector<AbstractNode*>{duplicate->eval->subscope->nodes[
						this->decision_input_node_contexts[i_index][0]->id]});
				new_branch_node->input_obs_indexes.push_back(this->decision_input_obs_indexes[i_index]);
			}
		}
	}
	for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
		for (int v_index = 0; v_index < (int)this->new_network_input_indexes[i_index].size(); v_index++) {
			int original_index = this->new_network_input_indexes[i_index][v_index];
			if (input_mapping[original_index] == -1) {
				input_mapping[original_index] = (int)new_branch_node->input_scope_contexts.size();
				new_branch_node->input_scope_context_ids.push_back(vector<int>{-1});
				new_branch_node->input_scope_contexts.push_back(
					vector<Scope*>{duplicate->eval->subscope});
				new_branch_node->input_node_context_ids.push_back(
					vector<int>{this->decision_input_node_contexts[original_index][0]->id});
				new_branch_node->input_node_contexts.push_back(
					vector<AbstractNode*>{duplicate->eval->subscope->nodes[
						this->decision_input_node_contexts[original_index][0]->id]});
				new_branch_node->input_obs_indexes.push_back(this->decision_input_obs_indexes[original_index]);
			}
		}
	}

	for (int i_index = 0; i_index < (int)this->existing_linear_weights.size(); i_index++) {
		if (this->existing_linear_weights[i_index] != 0.0) {
			new_branch_node->linear_original_input_indexes.push_back(input_mapping[i_index]);
			new_branch_node->linear_original_weights.push_back(this->existing_linear_weights[i_index]);
		}
	}
	for (int i_index = 0; i_index < (int)this->new_linear_weights.size(); i_index++) {
		if (this->new_linear_weights[i_index] != 0.0) {
			new_branch_node->linear_branch_input_indexes.push_back(input_mapping[i_index]);
			new_branch_node->linear_branch_weights.push_back(this->new_linear_weights[i_index]);
		}
	}

	for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
		vector<int> input_indexes;
		for (int v_index = 0; v_index < (int)this->existing_network_input_indexes[i_index].size(); v_index++) {
			input_indexes.push_back(input_mapping[this->existing_network_input_indexes[i_index][v_index]]);
		}
		new_branch_node->original_network_input_indexes.push_back(input_indexes);
	}
	new_branch_node->original_network = this->existing_network;
	this->existing_network = NULL;
	for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
		vector<int> input_indexes;
		for (int v_index = 0; v_index < (int)this->new_network_input_indexes[i_index].size(); v_index++) {
			input_indexes.push_back(input_mapping[this->new_network_input_indexes[i_index][v_index]]);
		}
		new_branch_node->branch_network_input_indexes.push_back(input_indexes);
	}
	new_branch_node->branch_network = this->new_network;
	this->new_network = NULL;

	AbstractNode* duplicate_explore_node = duplicate->eval->subscope->nodes[this->node_context->id];
	if (duplicate_explore_node->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)duplicate_explore_node;

		if (action_node->next_node == NULL) {
			/**
			 * - ending node edge case
			 *   - exit_node must be new ending node
			 */
			new_branch_node->original_next_node_id = exit_node_id;
			new_branch_node->original_next_node = exit_node;
		} else {
			new_branch_node->original_next_node_id = action_node->next_node_id;
			new_branch_node->original_next_node = action_node->next_node;
		}
	} else {
		BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

		if (this->is_branch) {
			new_branch_node->original_next_node_id = branch_node->branch_next_node_id;
			new_branch_node->original_next_node = branch_node->branch_next_node;
		} else {
			new_branch_node->original_next_node_id = branch_node->original_next_node_id;
			new_branch_node->original_next_node = branch_node->original_next_node;
		}
	}

	if (this->actions.size() == 0) {
		new_branch_node->branch_next_node_id = this->exit_next_node->id;
		new_branch_node->branch_next_node = duplicate->eval->subscope->nodes[this->exit_next_node->id];
	} else {
		new_branch_node->branch_next_node_id = this->actions[0]->id;
		new_branch_node->branch_next_node = this->actions[0];
	}

	if (duplicate_explore_node->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)duplicate_explore_node;

		action_node->next_node_id = new_branch_node->id;
		action_node->next_node = new_branch_node;
	} else if (duplicate_explore_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)duplicate_explore_node;

		scope_node->next_node_id = new_branch_node->id;
		scope_node->next_node = new_branch_node;
	} else {
		BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

		if (this->is_branch) {
			branch_node->branch_next_node_id = new_branch_node->id;
			branch_node->branch_next_node = new_branch_node;
		} else {
			branch_node->original_next_node_id = new_branch_node->id;
			branch_node->original_next_node = new_branch_node;
		}
	}

	this->actions.clear();
}

void EvalExperiment::new_pass_through(Solution* duplicate) {
	cout << "new_pass_through" << endl;

	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		this->actions[a_index]->parent = duplicate->eval->subscope;
		this->actions[a_index]->id = duplicate->eval->subscope->node_counter;
		duplicate->eval->subscope->node_counter++;
		duplicate->eval->subscope->nodes[this->actions[a_index]->id] = this->actions[a_index];
	}

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
		ActionNode* new_ending_node = new ActionNode();
		new_ending_node->parent = duplicate->eval->subscope;
		new_ending_node->id = duplicate->eval->subscope->node_counter;
		duplicate->eval->subscope->node_counter++;
		duplicate->eval->subscope->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->action = Action(ACTION_NOOP);

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (a_index == (int)this->actions.size()-1) {
			next_node_id = exit_node_id;
			next_node = exit_node;
		} else {
			next_node_id = this->actions[a_index+1]->id;
			next_node = this->actions[a_index+1];
		}

		this->actions[a_index]->next_node_id = next_node_id;
		this->actions[a_index]->next_node = next_node;
	}

	int start_node_id;
	AbstractNode* start_node;
	if (this->actions.size() == 0) {
		start_node_id = this->exit_next_node->id;
		start_node = duplicate->eval->subscope->nodes[this->exit_next_node->id];
	} else {
		start_node_id = this->actions[0]->id;
		start_node = this->actions[0];
	}

	AbstractNode* duplicate_explore_node = duplicate->eval->subscope->nodes[this->node_context->id];
	if (duplicate_explore_node->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)duplicate_explore_node;

		action_node->next_node_id = start_node_id;
		action_node->next_node = start_node;
	} else {
		BranchNode* branch_node = (BranchNode*)duplicate_explore_node;

		if (this->is_branch) {
			branch_node->branch_next_node_id = start_node_id;
			branch_node->branch_next_node = start_node;
		} else {
			branch_node->original_next_node_id = start_node_id;
			branch_node->original_next_node = start_node;
		}
	}

	this->actions.clear();
}
