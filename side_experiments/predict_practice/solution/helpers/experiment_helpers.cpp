#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void count_eval_helper(ScopeHistory* scope_history,
					   int& node_count,
					   int& eval_count) {
	if (scope_history->scope->is_outer) {
		return;
	}

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				count_eval_helper(scope_node_history->scope_history,
								  node_count,
								  eval_count);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				node_count++;
				if (obs_node->experiment != NULL) {
					eval_count++;
				}
			}
			break;
		}
	}
}

/**
 * - don't prioritize exploring new nodes as new scopes change explore
 */
void gather_helper(ScopeHistory* scope_history,
				   int& node_count,
				   ObsNode*& explore_node) {
	if (scope_history->scope->is_outer) {
		return;
	}

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				if (obs_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, node_count);
					node_count++;
					if (select_distribution(generator) == 0) {
						explore_node = obs_node;
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				gather_helper(scope_node_history->scope_history,
							  node_count,
							  explore_node);
			}
			break;
		}
	}
}

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper) {
	int node_count = 0;
	ObsNode* explore_node = NULL;
	gather_helper(scope_history,
				  node_count,
				  explore_node);

	if (explore_node != NULL) {
		Experiment* new_experiment = new Experiment(
			explore_node);
		explore_node->experiment = new_experiment;
	}
}
