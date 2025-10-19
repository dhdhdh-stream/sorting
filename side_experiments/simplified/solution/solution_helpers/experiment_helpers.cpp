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
 * - prioritize exploring new nodes
 * 
 * - even if changes made later, unlikely to influence old
 *   - old still needs to also match everything else
 */
void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)node;
				if (start_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					if (distribution(generator) <= 1.0 / (1.0 + sqrt(start_node->num_experiments))) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = start_node;
							explore_is_branch = false;
						}
					}
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (action_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					if (distribution(generator) <= 1.0 / (1.0 + sqrt(action_node->num_experiments))) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = action_node;
							explore_is_branch = false;
						}
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
					if (distribution(generator) <= 1.0 / (1.0 + sqrt(scope_node->num_experiments))) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = scope_node;
							explore_is_branch = false;
						}
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
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(branch_node->branch_num_experiments))) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = branch_node;
								explore_is_branch = true;
							}
						}
					}
				} else {
					if (branch_node->original_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
						if (distribution(generator) <= 1.0 / (1.0 + sqrt(branch_node->original_num_experiments))) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = branch_node;
								explore_is_branch = false;
							}
						}
					}
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				if (obs_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					if (distribution(generator) <= 1.0 / (1.0 + sqrt(obs_node->num_experiments))) {
						uniform_int_distribution<int> select_distribution(0, node_count);
						node_count++;
						if (select_distribution(generator) == 0) {
							explore_node = obs_node;
							explore_is_branch = false;
						}
					}
				}
			}
			break;
		}
	}
}

void create_experiment(SolutionWrapper* wrapper,
					   AbstractExperiment*& curr_experiment) {
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
				curr_experiment = new_experiment;

				switch (explore_node->type) {
				case NODE_TYPE_START:
					{
						StartNode* start_node = (StartNode*)explore_node;
						start_node->num_experiments++;
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)explore_node;
						action_node->num_experiments++;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)explore_node;
						scope_node->num_experiments++;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)explore_node;
						if (explore_is_branch) {
							branch_node->branch_num_experiments++;
						} else {
							branch_node->original_num_experiments++;
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)explore_node;
						obs_node->num_experiments++;
					}
					break;
				}
			} else {
				bool is_child = false;
				for (int c_index = 0; c_index < (int)explore_node->parent->child_scopes.size(); c_index++) {
					if (explore_node->parent->child_scopes[c_index] == wrapper->solution->last_new_scope) {
						is_child = true;
						break;
					}
				}
				if (is_child) {
					NewScopeExperiment* new_scope_experiment = new NewScopeExperiment(
						explore_node->parent,
						explore_node,
						explore_is_branch,
						wrapper->solution->last_new_scope,
						false);
					curr_experiment = new_scope_experiment;
				}
			}
		} else {
			if (wrapper->solution->timestamp % 10 == 5) {
				if (explore_node->parent->nodes.size() >= NEW_SCOPE_MIN_NODES) {
					Scope* new_scope = create_new_scope(explore_node->parent);
					if (new_scope != NULL) {
						NewScopeExperiment* new_scope_experiment = new NewScopeExperiment(
							explore_node->parent,
							explore_node,
							explore_is_branch,
							new_scope,
							true);
						curr_experiment = new_scope_experiment;
					}
				}
			} else {
				BranchExperiment* new_experiment = new BranchExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch,
					wrapper);
				curr_experiment = new_experiment;

				switch (explore_node->type) {
				case NODE_TYPE_START:
					{
						StartNode* start_node = (StartNode*)explore_node;
						start_node->num_experiments++;
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)explore_node;
						action_node->num_experiments++;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)explore_node;
						scope_node->num_experiments++;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)explore_node;
						if (explore_is_branch) {
							branch_node->branch_num_experiments++;
						} else {
							branch_node->original_num_experiments++;
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)explore_node;
						obs_node->num_experiments++;
					}
					break;
				}
			}
		}
	}
}

double get_experiment_impact(AbstractExperiment* experiment) {
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
