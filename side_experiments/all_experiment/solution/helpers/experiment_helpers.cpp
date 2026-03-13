#include "helpers.h"

#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_experiment.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void create_scope_experiment(SolutionWrapper* wrapper) {
	vector<ObsNode*> candidate;
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->average_hits_per_run >= SCOPE_EXPERIMENT_MIN_HIT) {
					candidate.push_back(obs_node);
				}
			}
		}
	}

	uniform_int_distribution<int> distribution(0, candidate.size()-1);
	ObsNode* node_context = candidate[distribution(generator)];

	Scope* new_scope = NULL;
	if (wrapper->solution->state == SOLUTION_STATE_OUTER) {
		outer_create_new_scope(wrapper,
							   new_scope);
	} else {
		create_new_scope(node_context->parent,
						 new_scope);
	}

	if (new_scope != NULL) {
		vector<AbstractNode*> possible_exits;
		node_context->parent->random_exit_activate(
			node_context->next_node,
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

		ScopeExperiment* scope_experiment = new ScopeExperiment(
			node_context,
			new_scope,
			exit_next_node);
		node_context->experiment = scope_experiment;

		wrapper->curr_scope_experiment = scope_experiment;
	}
}
