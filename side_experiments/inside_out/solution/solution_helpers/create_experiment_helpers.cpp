#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "commit_experiment.h"
#include "constants.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int NEW_SCOPE_MIN_NODES = 20;

void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   AbstractNode*& explore_node,
				   bool& explore_is_branch) {
	int num_nodes = (int)scope_history->scope->nodes.size();
	double scope_probability = 1.0 / sqrt((double)num_nodes);
	uniform_real_distribution<double> scope_distribution(0.0, 1.0);
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (action_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
						&& action_node->experiment == NULL) {
					if (scope_distribution(generator) <= scope_probability) {
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

				if (scope_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
						&& scope_node->experiment == NULL) {
					if (scope_distribution(generator) <= scope_probability) {
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
					if (branch_node->branch_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
							&& branch_node->experiment == NULL) {
						if (scope_distribution(generator) <= scope_probability) {
							uniform_int_distribution<int> select_distribution(0, node_count);
							node_count++;
							if (select_distribution(generator) == 0) {
								explore_node = branch_node;
								explore_is_branch = true;
							}
						}
					}
				} else {
					if (branch_node->original_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
							&& branch_node->experiment == NULL) {
						if (scope_distribution(generator) <= scope_probability) {
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
				if (obs_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
						&& obs_node->experiment == NULL) {
					if (scope_distribution(generator) <= scope_probability) {
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

void even_gather_helper(ScopeHistory* scope_history,
						map<Scope*, pair<int,pair<AbstractNode*,bool>>>& nodes_seen) {
	map<Scope*, pair<int,pair<AbstractNode*,bool>>>::iterator scope_it = nodes_seen.find(scope_history->scope);
	if (scope_it == nodes_seen.end()) {
		scope_it = nodes_seen.insert({scope_history->scope, {0, {NULL,false}}}).first;
	}
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				if (action_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
						&& action_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, scope_it->second.first);
					scope_it->second.first++;
					if (select_distribution(generator) == 0) {
						scope_it->second.second = {action_node, false};
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				even_gather_helper(scope_node_history->scope_history,
								   nodes_seen);

				if (scope_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
						&& scope_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, scope_it->second.first);
					scope_it->second.first++;
					if (select_distribution(generator) == 0) {
						scope_it->second.second = {scope_node, false};
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node;
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				if (branch_node_history->is_branch) {
					if (branch_node->branch_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
							&& branch_node->experiment == NULL) {
						uniform_int_distribution<int> select_distribution(0, scope_it->second.first);
						scope_it->second.first++;
						if (select_distribution(generator) == 0) {
							scope_it->second.second = {branch_node, true};
						}
					}
				} else {
					if (branch_node->original_average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
							&& branch_node->experiment == NULL) {
						uniform_int_distribution<int> select_distribution(0, scope_it->second.first);
						scope_it->second.first++;
						if (select_distribution(generator) == 0) {
							scope_it->second.second = {branch_node, false};
						}
					}
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				if (obs_node->average_hits_per_run > EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN
						&& obs_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, scope_it->second.first);
					scope_it->second.first++;
					if (select_distribution(generator) == 0) {
						scope_it->second.second = {obs_node, false};
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

	AbstractNode* explore_node = NULL;
	bool explore_is_branch = false;
	uniform_int_distribution<int> even_distribution(0, 1);
	if (even_distribution(generator) == 0) {
		/**
		 * - to try to give outer scopes extra weight
		 *   - as outer scopes can have bigger impact through child scopes
		 */
		map<Scope*, pair<int,pair<AbstractNode*,bool>>> nodes_seen;
		even_gather_helper(scope_history,
						   nodes_seen);

		bool is_selected = false;
		uniform_real_distribution<double> scope_distribution(0.0, 1.0);
		while (!is_selected) {
			int scope_count = 0;
			for (map<Scope*, pair<int,pair<AbstractNode*,bool>>>::iterator it = nodes_seen.begin();
					it != nodes_seen.end(); it++) {
				int num_nodes = (int)it->first->nodes.size();
				double scope_probability = 1.0 / sqrt((double)num_nodes);
				if (scope_distribution(generator) <= scope_probability) {
					uniform_int_distribution<int> select_distribution(0, scope_count);
					scope_count++;
					if (select_distribution(generator) == 0) {
						explore_node = it->second.second.first;
						explore_is_branch = it->second.second.second;
						is_selected = true;
					}
				}
			}
		}
	} else {
		int node_count = 0;
		gather_helper(scope_history,
					  node_count,
					  explore_node,
					  explore_is_branch);
	}

	if (explore_node != NULL) {
		uniform_int_distribution<int> non_new_distribution(0, 9);
		if (explore_node->parent->nodes.size() >= NEW_SCOPE_MIN_NODES
				&& non_new_distribution(generator) != 0) {
			NewScopeExperiment* new_scope_experiment = new NewScopeExperiment(
				explore_node->parent,
				explore_node,
				explore_is_branch);

			if (new_scope_experiment->result == EXPERIMENT_RESULT_FAIL) {
				delete new_scope_experiment;
			} else {
				curr_experiment = new_scope_experiment;
			}
		} else {
			uniform_int_distribution<int> commit_distribution(0, 9);
			if (explore_node->parent->nodes.size() < 20
					&& commit_distribution(generator) == 0) {
				CommitExperiment* new_commit_experiment = new CommitExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch);

				if (new_commit_experiment->result == EXPERIMENT_RESULT_FAIL) {
					delete new_commit_experiment;
				} else {
					curr_experiment = new_commit_experiment;
				}
			} else {
				BranchExperiment* new_experiment = new BranchExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch);

				if (new_experiment->result == EXPERIMENT_RESULT_FAIL) {
					delete new_experiment;
				} else {
					curr_experiment = new_experiment;
				}
			}
		}
	}
}
