#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void create_experiment(SolutionWrapper* wrapper) {
	vector<Scope*> possible_scopes;
	for (map<int, Scope*>::iterator it = wrapper->solution->scopes.begin();
			it != wrapper->solution->scopes.end(); it++) {
		if (it->second->average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
			possible_scopes.push_back(it->second);
		}
	}

	uniform_int_distribution<int> explore_scope_distribution(0, possible_scopes.size()-1);
	Scope* explore_scope = possible_scopes[explore_scope_distribution(generator)];

	vector<pair<AbstractNode*,bool>> possible_explore_nodes;
	for (map<int, AbstractNode*>::iterator it = explore_scope->nodes.begin();
			it != explore_scope->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_START:
			possible_explore_nodes.push_back({it->second, false});
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second;
				if (action_node->average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({action_node, false});
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)it->second;
				if (scope_node->average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({scope_node, false});
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				if (branch_node->original_average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({branch_node, false});
				}
				if (branch_node->branch_average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({branch_node, true});
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->average_hits_per_run >= EXPERIMENT_MIN_AVERAGE_HITS_PER_RUN) {
					possible_explore_nodes.push_back({obs_node, false});
				}
			}
			break;
		}
	}

	uniform_int_distribution<int> explore_distribution(0, possible_explore_nodes.size()-1);
	int random_index = explore_distribution(generator);
	AbstractNode* explore_node = possible_explore_nodes[random_index].first;
	bool explore_is_branch = possible_explore_nodes[random_index].second;

	BranchExperiment* new_experiment = new BranchExperiment(
		explore_node->parent,
		explore_node,
		explore_is_branch,
		wrapper);

	if (new_experiment->result == EXPERIMENT_RESULT_FAIL) {
		delete new_experiment;
	} else {
		wrapper->curr_experiment = new_experiment;
	}
}
