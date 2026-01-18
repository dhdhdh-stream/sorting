#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void gather_helper(ScopeHistory* scope_history,
				   ObsNode*& node_context,
				   int& potential_seen) {
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				if (obs_node->experiment == NULL) {
					uniform_int_distribution<int> select_distribution(0, potential_seen);
					if (select_distribution(generator) == 0) {
						node_context = obs_node;
					}
					potential_seen++;
				}
			}
			break;
		}
	}
}

void create_experiment(SolutionWrapper* wrapper) {
	ObsNode* node_context = NULL;
	int potential_seen = 0;
	gather_helper(wrapper->scope_histories[0],
				  node_context,
				  potential_seen);

	if (node_context != NULL) {
		Network* network = NULL;
		bool is_success = train_existing_helper(node_context,
												wrapper,
												network);
		if (is_success) {
			Scope* scope_context = node_context->parent;

			vector<AbstractNode*> possible_exits;

			AbstractNode* starting_node = node_context->next_node;
			scope_context->random_exit_activate(
				starting_node,
				possible_exits);

			int random_index;
			geometric_distribution<int> exit_distribution(0.1);
			while (true) {
				random_index = exit_distribution(generator);
				if (random_index < (int)possible_exits.size()) {
					break;
				}
			}
			AbstractNode* exit_next_node = possible_exits[random_index];

			ExploreExperiment* new_experiment = new ExploreExperiment(
				node_context,
				exit_next_node,
				network);
			node_context->experiment = new_experiment;
		}
	}
}
