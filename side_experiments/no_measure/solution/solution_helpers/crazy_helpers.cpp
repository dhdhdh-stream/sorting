#include "solution_helpers.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"
#include "utilities.h"

using namespace std;

void add_crazy_helper(Scope* scope_context,
					  AbstractNode* node_context,
					  bool is_branch,
					  AbstractNode* exit_next_node,
					  SolutionWrapper* wrapper) {
	std::vector<int> step_types;
	std::vector<int> actions;
	std::vector<Scope*> scopes;

	bool exit_is_next;
	switch (node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)node_context;
			if (exit_next_node == start_node->next_node) {
				exit_is_next = true;
			} else {
				exit_is_next = false;
			}
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			if (exit_next_node == action_node->next_node) {
				exit_is_next = true;
			} else {
				exit_is_next = false;
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)node_context;
			if (exit_next_node == scope_node->next_node) {
				exit_is_next = true;
			} else {
				exit_is_next = false;
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				if (exit_next_node == branch_node->branch_next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			} else {
				if (exit_next_node == branch_node->original_next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			}
		}
		break;
	default:
	// case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)node_context;
			if (exit_next_node == obs_node->next_node) {
				exit_is_next = true;
			} else {
				exit_is_next = false;
			}
		}
		break;
	}

	int new_num_steps;
	geometric_distribution<int> geo_distribution(0.3);
	/**
	 * - num_steps less than exit length on average to reduce solution size
	 */
	if (exit_is_next) {
		new_num_steps = 1 + geo_distribution(generator);
	} else {
		new_num_steps = geo_distribution(generator);
	}

	vector<int> possible_child_indexes;
	for (int c_index = 0; c_index < (int)node_context->parent->child_scopes.size(); c_index++) {
		if (node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
			possible_child_indexes.push_back(c_index);
		}
	}
	uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		bool is_scope = false;
		if (possible_child_indexes.size() > 0) {
			if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
				uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
				if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
					is_scope = true;
				}
			} else {
				uniform_int_distribution<int> scope_distribution(0, 1);
				if (scope_distribution(generator) == 0) {
					is_scope = true;
				}
			}
		}
		if (is_scope) {
			step_types.push_back(STEP_TYPE_SCOPE);
			actions.push_back(-1);

			int child_index = possible_child_indexes[child_index_distribution(generator)];
			scopes.push_back(node_context->parent->child_scopes[child_index]);
		} else {
			step_types.push_back(STEP_TYPE_ACTION);

			uniform_int_distribution<int> action_distribution(0, 6);
			actions.push_back(action_distribution(generator));

			scopes.push_back(NULL);
		}
	}

	stringstream ss;
	ss << get_time() << "; ";
	ss << "timestamp: " << wrapper->solution->timestamp << "; ";
	ss << "Crazy" << "; ";
	ss << "scope_context->id: " << scope_context->id << "; ";
	ss << "node_context->id: " << node_context->id << "; ";
	ss << "is_branch: " << is_branch << "; ";
	ss << "new explore path:";
	for (int s_index = 0; s_index < (int)step_types.size(); s_index++) {
		if (step_types[s_index] == STEP_TYPE_ACTION) {
			ss << " " << actions[s_index];
		} else {
			ss << " E" << scopes[s_index]->id;
		}
	}
	ss << "; ";

	if (exit_next_node == NULL) {
		ss << "exit_next_node->id: " << -1 << "; ";
	} else {
		ss << "exit_next_node->id: " << exit_next_node->id << "; ";
	}

	double previous_val_average = measure_helper(wrapper);
	wrapper->solution->improvement_history.push_back(previous_val_average);
	cout << "previous_val_average: " << previous_val_average << endl;

	wrapper->solution->curr_score = previous_val_average;

	wrapper->solution->change_history.push_back(ss.str());

	cout << ss.str() << endl;

	vector<AbstractNode*> new_nodes;
	for (int s_index = 0; s_index < (int)step_types.size(); s_index++) {
		if (step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = scope_context;
			new_action_node->id = scope_context->node_counter;
			scope_context->node_counter++;
			scope_context->nodes[new_action_node->id] = new_action_node;

			new_action_node->action = actions[s_index];

			new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = scope_context;
			new_scope_node->id = scope_context->node_counter;
			scope_context->node_counter++;
			scope_context->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->scope = scopes[s_index];

			new_nodes.push_back(new_scope_node);
		}
	}

	ObsNode* new_ending_node = NULL;

	int exit_node_id;
	AbstractNode* exit_node;
	if (exit_next_node == NULL) {
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
		exit_node_id = exit_next_node->id;
		exit_node = exit_next_node;
	}

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->parent = scope_context;
	new_branch_node->id = scope_context->node_counter;
	scope_context->node_counter++;
	scope_context->nodes[new_branch_node->id] = new_branch_node;

	switch (node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)node_context;

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
			ActionNode* action_node = (ActionNode*)node_context;

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
			ScopeNode* scope_node = (ScopeNode*)node_context;

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
			BranchNode* branch_node = (BranchNode*)node_context;

			if (is_branch) {
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
			ObsNode* obs_node = (ObsNode*)node_context;

			if (obs_node->next_node == NULL) {
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

	if (step_types.size() == 0) {
		exit_node->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->branch_next_node_id = exit_node_id;
		new_branch_node->branch_next_node = exit_node;
	} else {
		new_nodes[0]->ancestor_ids.push_back(new_branch_node->id);

		new_branch_node->branch_next_node_id = new_nodes[0]->id;
		new_branch_node->branch_next_node = new_nodes[0];
	}

	switch (node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)node_context;

			start_node->next_node_id = new_branch_node->id;
			start_node->next_node = new_branch_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;

			action_node->next_node_id = new_branch_node->id;
			action_node->next_node = new_branch_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)node_context;

			scope_node->next_node_id = new_branch_node->id;
			scope_node->next_node = new_branch_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;

			if (is_branch) {
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
			ObsNode* obs_node = (ObsNode*)node_context;

			obs_node->next_node_id = new_branch_node->id;
			obs_node->next_node = new_branch_node;
		}
		break;
	}
	new_branch_node->ancestor_ids.push_back(node_context->id);

	vector<double> means(25, 0.0);
	vector<double> deviations(25, 1.0);
	Network* existing_network = new Network(25,
											means,
											deviations);
	new_branch_node->original_network = existing_network;
	Network* new_network = new Network(25,
									   means,
									   deviations);
	new_branch_node->branch_network = new_network;

	new_branch_node->ramp = 0;
	new_branch_node->ramp_num_gears = 20;
	new_branch_node->ramp_iter = 0;

	new_branch_node->consec_original = 0;
	new_branch_node->consec_branch = 0;

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

	clean_scope(scope_context);

	wrapper->solution->timestamp++;
	// if ((int)wrapper->solution->improvement_history.size() >= STUCK_NUM_ITERS) {
	// 	double prev_val = wrapper->solution->improvement_history[wrapper->solution->improvement_history.size() - STUCK_NUM_ITERS];
	// 	bool improved = false;
	// 	for (int h_index = 0; h_index < STUCK_NUM_ITERS-1; h_index++) {
	// 		if (wrapper->solution->improvement_history[wrapper->solution->improvement_history.size() - 1 - h_index] > prev_val) {
	// 			improved = true;
	// 			break;
	// 		}
	// 	}

	// 	if (!improved) {
	// 		wrapper->solution->timestamp = -1;
	// 	}
	// }
	// // temp
	// if (wrapper->solution->timestamp >= 40) {
	// 	wrapper->solution->timestamp = -1;
	// }

	if (scope_context == wrapper->solution->starting_scope) {
		wrapper->solution->starting_num_improvements++;
		if (wrapper->solution->starting_num_improvements >= GENERALIZE_ITER) {
			Scope* new_scope = new Scope();
			new_scope->id = wrapper->solution->scopes.size();
			new_scope->node_counter = 0;
			wrapper->solution->scopes.push_back(new_scope);

			new_scope->child_scopes = wrapper->solution->starting_scope->child_scopes;
			new_scope->child_scopes.push_back(wrapper->solution->starting_scope);

			new_scope->last_scores = wrapper->solution->starting_scope->last_scores;

			StartNode* start_node = new StartNode();
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

			ObsNode* end_node = new ObsNode();
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

	/**
	 * - reset all other experiments
	 */
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->experiment != NULL) {
				delete it->second->experiment;
				it->second->experiment = NULL;
			}
		}
	}
	wrapper->experiment_iter = 0;
}
