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

void create_experiment(ObsNode* node,
					   SolutionWrapper* wrapper) {
	AbstractNode* starting_node = node->next_node;
	vector<AbstractNode*> possible_exits;
	node->parent->random_exit_activate(
		starting_node,
		possible_exits);

	geometric_distribution<int> exit_distribution(0.1);
	int random_index;
	while (true) {
		random_index = exit_distribution(generator);
		if (random_index < (int)possible_exits.size()) {
			break;
		}
	}
	AbstractNode* exit_next_node = possible_exits[random_index];

	Experiment* new_experiment = new Experiment(
		node,
		exit_next_node);
	node->experiment = new_experiment;
}

void create_experiments(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->experiment != NULL) {
					create_experiment(obs_node,
									  wrapper);
				}
			}
		}
	}
}
