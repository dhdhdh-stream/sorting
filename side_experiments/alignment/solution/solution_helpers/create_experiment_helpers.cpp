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
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

const double EXPERIMENT_MIN_AVERAGE_INSTANCES_PER_RUN = 0.5;

const int MERGE_MIN_FOLLOW_NODES = 5;

void gather_nodes_seen_helper(ScopeHistory* scope_history,
							  map<pair<AbstractNode*,bool>, int>& nodes_seen) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		if (h_it->second->node->experiment == NULL
				&& h_it->second->node->average_instances_per_run > EXPERIMENT_MIN_AVERAGE_INSTANCES_PER_RUN) {
			switch (h_it->second->node->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_OBS:
				{
					map<pair<AbstractNode*,bool>, int>::iterator seen_it = nodes_seen
						.find({h_it->second->node, false});
					if (seen_it == nodes_seen.end()) {
						nodes_seen[{h_it->second->node, false}] = 1;
					} else {
						seen_it->second++;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

					gather_nodes_seen_helper(scope_node_history->scope_history,
											 nodes_seen);

					map<pair<AbstractNode*,bool>, int>::iterator seen_it = nodes_seen
						.find({h_it->second->node, false});
					if (seen_it == nodes_seen.end()) {
						nodes_seen[{h_it->second->node, false}] = 1;
					} else {
						seen_it->second++;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
					map<pair<AbstractNode*,bool>, int>::iterator seen_it = nodes_seen
						.find({h_it->second->node, branch_node_history->is_branch});
					if (seen_it == nodes_seen.end()) {
						nodes_seen[{h_it->second->node, branch_node_history->is_branch}] = 1;
					} else {
						seen_it->second++;
					}
				}
				break;
			}
		}
	}
}

void gather_ancestors_helper(AbstractNode* curr_node,
							 set<AbstractNode*>& ancestors) {
	for (int a_index = 0; a_index < (int)curr_node->ancestor_ids.size(); a_index++) {
		AbstractNode* ancestor = curr_node->parent->nodes[curr_node->ancestor_ids[a_index]];
		set<AbstractNode*>::iterator it = ancestors.find(ancestor);
		if (it == ancestors.end()) {
			ancestors.insert(ancestor);
			gather_ancestors_helper(ancestor,
									ancestors);
		}
	}
}

void gather_children_helper(AbstractNode* curr_node,
							set<AbstractNode*>& children) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)curr_node;
			set<AbstractNode*>::iterator it = children.find(action_node->next_node);
			if (it == children.end()) {
				children.insert(action_node->next_node);
				gather_children_helper(action_node->next_node,
									   children);
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)curr_node;
			set<AbstractNode*>::iterator it = children.find(scope_node->next_node);
			if (it == children.end()) {
				children.insert(scope_node->next_node);
				gather_children_helper(scope_node->next_node,
									   children);
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)curr_node;
			set<AbstractNode*>::iterator original_it = children.find(branch_node->original_next_node);
			if (original_it == children.end()) {
				children.insert(branch_node->original_next_node);
				gather_children_helper(branch_node->original_next_node,
									   children);
			}
			set<AbstractNode*>::iterator branch_it = children.find(branch_node->branch_next_node);
			if (branch_it == children.end()) {
				children.insert(branch_node->branch_next_node);
				gather_children_helper(branch_node->branch_next_node,
									   children);
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)curr_node;
			set<AbstractNode*>::iterator it = children.find(obs_node->next_node);
			if (it == children.end()) {
				children.insert(obs_node->next_node);
				gather_children_helper(obs_node->next_node,
									   children);
			}
		}
		break;
	}
}

void create_experiment(ScopeHistory* scope_history,
					   int improvement_iter,
					   AbstractExperiment*& curr_experiment) {
	map<pair<AbstractNode*,bool>,int> nodes_seen;
	gather_nodes_seen_helper(scope_history,
							 nodes_seen);

	if (nodes_seen.size() > 0) {
		AbstractNode* explore_node;
		bool explore_is_branch;
		uniform_int_distribution<int> even_distribution(0, 1);
		if (even_distribution(generator) == 0) {
			uniform_int_distribution<int> explore_node_distribution(0, nodes_seen.size()-1);
			int explore_node_index = explore_node_distribution(generator);
			map<pair<AbstractNode*,bool>, int>::iterator it = next(nodes_seen.begin(), explore_node_index);
			explore_node = it->first.first;
			explore_is_branch = it->first.second;
		} else {
			int sum_count = 0;
			for (map<pair<AbstractNode*,bool>, int>::iterator it = nodes_seen.begin();
					it != nodes_seen.end(); it++) {
				sum_count += it->second;
			}
			uniform_int_distribution<int> random_distribution(1, sum_count);
			int random_index = random_distribution(generator);
			for (map<pair<AbstractNode*,bool>, int>::iterator it = nodes_seen.begin();
					it != nodes_seen.end(); it++) {
				random_index -= it->second;
				if (random_index <= 0) {
					explore_node = it->first.first;
					explore_is_branch = it->first.second;
					break;
				}
			}
		}
		/**
		 * - don't weigh based on number of nodes within scope
		 *   - can get trapped by small useless scopes
		 *     - may be good for certain decision heavy scopes to have lots of nodes
		 */

		set<AbstractNode*> ancestors;
		gather_ancestors_helper(explore_node,
								ancestors);
		set<AbstractNode*> possible_exit_next_nodes;
		for (map<int, AbstractNode*>::iterator it = explore_node->parent->nodes.begin();
				it != explore_node->parent->nodes.end(); it++) {
			possible_exit_next_nodes.insert(it->second);
		}
		for (set<AbstractNode*>::iterator it = ancestors.begin();
				it != ancestors.begin(); it++) {
			possible_exit_next_nodes.erase(*it);
		}
		uniform_int_distribution<int> exit_next_node_distribution(0, possible_exit_next_nodes.size()-1);
		AbstractNode* potential_exit_next_node = *next(possible_exit_next_nodes.begin(), exit_next_node_distribution(generator));
		set<AbstractNode*> children;
		gather_children_helper(potential_exit_next_node,
							   children);
		int num_meaningful_nodes = 0;
		for (set<AbstractNode*>::iterator it = children.begin();
				it != children.end(); it++) {
			switch ((*it)->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_SCOPE:
				num_meaningful_nodes++;
				break;
			}
		}
		if (num_meaningful_nodes < MERGE_MIN_FOLLOW_NODES) {
			potential_exit_next_node = NULL;
		}

		if (explore_node->parent->exceeded) {
			uniform_int_distribution<int> non_new_distribution(0, 1);
			if (non_new_distribution(generator) != 0) {
				NewScopeExperiment* new_scope_experiment = new NewScopeExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch,
					potential_exit_next_node);

				if (new_scope_experiment->result == EXPERIMENT_RESULT_FAIL) {
					delete new_scope_experiment;
				} else {
					explore_node->experiment = new_scope_experiment;

					curr_experiment = new_scope_experiment;
				}
			} else {
				PassThroughExperiment* new_experiment = new PassThroughExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch,
					potential_exit_next_node);

				if (new_experiment->result == EXPERIMENT_RESULT_FAIL) {
					delete new_experiment;
				} else {
					explore_node->experiment = new_experiment;

					curr_experiment = new_experiment;
				}
			}
		} else {
			/**
			 * - weigh towards PassThroughExperiments as cheaper and potentially just as effective
			 *   - solutions are often made of relatively few distinct decisions, but applied such that has good coverage
			 *     - like tessellation, but have to get both the shape and the pattern correct
			 *       - and PassThroughExperiments help with both
			 */
			if (improvement_iter == 0) {
				CommitExperiment* new_commit_experiment = new CommitExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch,
					potential_exit_next_node);
				explore_node->experiment = new_commit_experiment;

				curr_experiment = new_commit_experiment;
			} else {
				BranchExperiment* new_experiment = new BranchExperiment(
					explore_node->parent,
					explore_node,
					explore_is_branch,
					potential_exit_next_node);

				explore_node->experiment = new_experiment;

				curr_experiment = new_experiment;
			}
		}
	}
}
