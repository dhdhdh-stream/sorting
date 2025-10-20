#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

/**
 * - don't prioritize exploring new nodes as new scopes change explore
 */
void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		if (node->experiment == NULL) {
			switch (node->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)node;
					if (start_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = start_node;
							explore_is_branch = false;
						}
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)node;
					if (action_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = action_node;
							explore_is_branch = false;
						}
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)node;
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

					gather_helper(scope_node_history->scope_history,
								  node_count,
								  explore_node,
								  explore_is_branch);

					if (scope_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = scope_node;
							explore_is_branch = false;
						}
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)node;
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
					if (branch_node_history->is_branch) {
						if (branch_node->branch_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = branch_node;
								explore_is_branch = true;
							}
						}
					} else {
						if (branch_node->original_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = branch_node;
								explore_is_branch = false;
							}
						}
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)node;
					if (obs_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = obs_node;
							explore_is_branch = false;
						}
					}
				}
				break;
			}
		}
	}
}

void create_branch_experiment(SolutionWrapper* wrapper) {
	uniform_int_distribution<int> scope_history_distribution(0,
		wrapper->solution->existing_scope_histories.size()-1);
	ScopeHistory* scope_history = wrapper->solution->existing_scope_histories[scope_history_distribution(generator)];

	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch);

	if (explore_node != NULL) {
		if (wrapper->solution->last_new_scope != NULL) {
			if (explore_node->parent == wrapper->solution->last_new_scope) {
				BranchExperiment* new_experiment = new BranchExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch,
					wrapper);
				wrapper->curr_branch_experiment = new_experiment;
			}
		} else {
			BranchExperiment* new_experiment = new BranchExperiment(
				explore_node->parent,
				explore_node,
				explore_is_branch,
				wrapper);
			wrapper->curr_branch_experiment = new_experiment;
		}
	}
}

void create_new_scope_overall_experiment(SolutionWrapper* wrapper) {
	uniform_int_distribution<int> scope_history_distribution(0,
		wrapper->solution->existing_scope_histories.size()-1);
	ScopeHistory* scope_history = wrapper->solution->existing_scope_histories[scope_history_distribution(generator)];

	int node_count = 0;
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	gather_helper(scope_history,
				  node_count,
				  explore_node,
				  explore_is_branch);

	if (explore_node != NULL) {
		if (explore_node->parent->nodes.size() >= NEW_SCOPE_MIN_NODES) {
			Scope* new_scope = create_new_scope(explore_node->parent);
			if (new_scope != NULL) {
				wrapper->curr_new_scope_experiment = new NewScopeOverallExperiment(
					new_scope,
					explore_node->parent);
			}
		}
	}
}

void new_scope_gather_helper(ScopeHistory* scope_history,
							 int& node_count,
							 AbstractNode*& explore_node,
							 bool& explore_is_branch,
							 Scope* scope_context) {
	Scope* scope = scope_history->scope;

	if (scope == scope_context) {
		for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
				h_it != scope_history->node_histories.end(); h_it++) {
			AbstractNode* node = h_it->second->node;
			if (node->experiment == NULL) {
				switch (node->type) {
				case NODE_TYPE_START:
					{
						StartNode* start_node = (StartNode*)node;
						if (start_node->new_scope_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = start_node;
								explore_is_branch = false;
							}
						}
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)node;
						if (action_node->new_scope_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = action_node;
								explore_is_branch = false;
							}
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)node;
						if (scope_node->new_scope_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = scope_node;
								explore_is_branch = false;
							}
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)node;
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
						if (branch_node_history->is_branch) {
							if (branch_node->new_scope_branch_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
								uniform_int_distribution<int> select_distribution(0, node_count);
								node_count++;
								if (select_distribution(generator) == 0) {
									explore_node = branch_node;
									explore_is_branch = true;
								}
							}
						} else {
							if (branch_node->new_scope_original_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
								uniform_int_distribution<int> select_distribution(0, node_count);
								node_count++;
								if (select_distribution(generator) == 0) {
									explore_node = branch_node;
									explore_is_branch = false;
								}
							}
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)node;
						if (obs_node->new_scope_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = obs_node;
								explore_is_branch = false;
							}
						}
					}
					break;
				}
			}
		}
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == scope_context) {
				is_child = true;
				break;
			}
		}
		if (is_child) {
			for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
					h_it != scope_history->node_histories.end(); h_it++) {
				AbstractNode* node = h_it->second->node;
				if (node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
					new_scope_gather_helper(scope_node_history->scope_history,
											node_count,
											explore_node,
											explore_is_branch,
											scope_context);
				}
			}
		}
	}
}

void create_new_scope_experiment(SolutionWrapper* wrapper) {
	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	if (wrapper->curr_new_scope_experiment->successful_experiments.size() == 0) {
		uniform_int_distribution<int> scope_history_distribution(0,
			wrapper->solution->existing_scope_histories.size()-1);
		ScopeHistory* scope_history = wrapper->solution->existing_scope_histories[scope_history_distribution(generator)];

		int node_count = 0;
		gather_helper(scope_history,
					  node_count,
					  explore_node,
					  explore_is_branch);
	} else {
		uniform_int_distribution<int> scope_history_distribution(0,
			wrapper->curr_new_scope_experiment->new_scope_histories.size()-1);
		ScopeHistory* scope_history = wrapper->curr_new_scope_experiment->new_scope_histories[scope_history_distribution(generator)];

		int node_count = 0;
		new_scope_gather_helper(scope_history,
								node_count,
								explore_node,
								explore_is_branch,
								wrapper->curr_new_scope_experiment->scope_context);
	}

	if (explore_node != NULL) {
		if (explore_node->parent == wrapper->curr_new_scope_experiment->scope_context) {
			NewScopeExperiment* new_experiment = new NewScopeExperiment(
				explore_node->parent,
				explore_node,
				explore_is_branch,
				wrapper->curr_new_scope_experiment->new_scope);
			wrapper->curr_new_scope_experiment->curr_experiment = new_experiment;
		}
	}
}

bool still_instances_possible_helper(ScopeHistory* scope_history,
									 Scope* scope_context) {
	Scope* scope = scope_history->scope;

	if (scope == scope_context) {
		for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
				h_it != scope_history->node_histories.end(); h_it++) {
			AbstractNode* node = h_it->second->node;
			if (node->experiment == NULL) {
				switch (node->type) {
				case NODE_TYPE_START:
					{
						StartNode* start_node = (StartNode*)node;
						if (start_node->new_scope_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							return true;
						}
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)node;
						if (action_node->new_scope_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							return true;
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)node;
						if (scope_node->new_scope_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							return true;
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)node;
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
						if (branch_node_history->is_branch) {
							if (branch_node->new_scope_branch_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
								return true;
							}
						} else {
							if (branch_node->new_scope_original_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
								return true;
							}
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)node;
						if (obs_node->new_scope_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
							return true;
						}
					}
					break;
				}
			}
		}
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == scope_context) {
				is_child = true;
				break;
			}
		}
		if (is_child) {
			for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
					h_it != scope_history->node_histories.end(); h_it++) {
				AbstractNode* node = h_it->second->node;
				if (node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
					bool inner_result = still_instances_possible_helper(
						scope_node_history->scope_history,
						scope_context);
					if (inner_result) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool still_instances_possible(NewScopeOverallExperiment* experiment) {
	uniform_int_distribution<int> distribution(0, experiment->new_scope_histories.size()-1);
	for (int t_index = 0; t_index < 10; t_index++) {
		int index = distribution(generator);
		bool curr_result = still_instances_possible_helper(
			experiment->new_scope_histories[index],
			experiment->scope_context);
		if (curr_result) {
			return true;
		}
	}

	return false;
}

double get_experiment_impact(BranchExperiment* experiment) {
	double existing_score;
	switch (experiment->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)experiment->node_context;
			existing_score = start_node->average_score;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)experiment->node_context;
			existing_score = action_node->average_score;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)experiment->node_context;
			existing_score = scope_node->average_score;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)experiment->node_context;
			if (experiment->is_branch) {
				existing_score = branch_node->branch_average_score;
			} else {
				existing_score = branch_node->original_average_score;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)experiment->node_context;
			existing_score = obs_node->average_score;
		}
		break;
	}

	return experiment->new_score - existing_score;
}

double get_experiment_impact(NewScopeOverallExperiment* experiment) {
	double sum_score = 0.0;
	for (int h_index = 0; h_index < (int)experiment->new_target_val_histories.size(); h_index++) {
		sum_score += experiment->new_target_val_histories[h_index];
	}

	return sum_score;
}
