#include "helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "start_node.h"

using namespace std;

Explore* create_explore(Scope* scope) {
	Explore* new_explore = new Explore();

	vector<pair<AbstractNode*,bool>> possible_explore_nodes;
	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
			it != scope->nodes.end(); it++) {
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
	int explore_random_index = explore_distribution(generator);
	new_explore->explore_node = possible_explore_nodes[explore_random_index].first;
	new_explore->explore_is_branch = possible_explore_nodes[explore_random_index].second;

	vector<AbstractNode*> possible_exits;

	AbstractNode* starting_node;
	switch (new_explore->explore_node->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)new_explore->explore_node;
			starting_node = start_node->next_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)new_explore->explore_node;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)new_explore->explore_node;
			starting_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)new_explore->explore_node;
			if (new_explore->explore_is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)new_explore->explore_node;
			starting_node = obs_node->next_node;
		}
		break;
	}

	scope->random_exit_activate(
		starting_node,
		possible_exits);

	int exit_random_index;
	geometric_distribution<int> exit_distribution(0.2);
	while (true) {
		exit_random_index = exit_distribution(generator);
		if (exit_random_index < (int)possible_exits.size()) {
			break;
		}
	}
	new_explore->exit_next_node = possible_exits[exit_random_index];

	#if defined(MDEBUG) && MDEBUG
	uniform_int_distribution<int> new_scope_distribution(0, 1);
	#else
	uniform_int_distribution<int> new_scope_distribution(0, 4);
	#endif /* MDEBUG */
	if (new_scope_distribution(generator) == 0) {
		new_explore->new_scope = create_new_scope(scope);
	}
	if (new_explore->new_scope != NULL) {
		new_explore->step_types.push_back(STEP_TYPE_SCOPE);
		new_explore->actions.push_back(-1);
		new_explore->scopes.push_back(new_explore->new_scope);
	} else {
		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.2);
		if (exit_random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> scope_distribution(0, 1);
		vector<Scope*> possible_scopes;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index]->nodes.size() > 1) {
				possible_scopes.push_back(scope->child_scopes[c_index]);
			}
		}
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (scope_distribution(generator) == 0 && possible_scopes.size() > 0) {
				new_explore->step_types.push_back(STEP_TYPE_SCOPE);
				new_explore->actions.push_back(-1);

				uniform_int_distribution<int> child_scope_distribution(0, possible_scopes.size()-1);
				new_explore->scopes.push_back(possible_scopes[child_scope_distribution(generator)]);
			} else {
				new_explore->step_types.push_back(STEP_TYPE_ACTION);

				new_explore->actions.push_back(-1);

				new_explore->scopes.push_back(NULL);
			}
		}
	}

	return new_explore;
}
