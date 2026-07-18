#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_experiment.h"
#include "globals.h"
#include "multi_experiment.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int MULTI_SCOPE_MIN_NODES = 10;

const int MULTI_TARGET_SIZE = 3;
const int MULTI_RANDOM_NUM_TRIES = 50;

class ExploreContext {
public:
	int node_count;

	AbstractNode* explore_node;
	bool explore_is_branch;
	vector<AbstractNode*> explore_node_histories;
	int explore_index;
};

/**
 * - don't prioritize exploring new nodes as new scopes change explore
 */
void gather_helper(ScopeHistory* scope_history,
				   map<Scope*, ExploreContext>& explore_contexts) {
	map<Scope*, ExploreContext>::iterator context_it = explore_contexts.find(scope_history->scope);
	if (context_it == explore_contexts.end()) {
		context_it = explore_contexts.insert({scope_history->scope, ExploreContext()}).first;
		context_it->second.node_count = 0;
		context_it->second.explore_node = NULL;
	}

	vector<AbstractNode*> curr_node_histories(scope_history->node_histories.size());
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		curr_node_histories[h_it->second->index] = h_it->second->node;
	}
	curr_node_histories.push_back(NULL);

	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		AbstractNode* node = h_it->second->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
		case NODE_TYPE_NOOP:
			{
				uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
				context_it->second.node_count++;
				if (select_distribution(generator) == 0) {
					context_it->second.explore_node = node;
					context_it->second.explore_is_branch = false;
					context_it->second.explore_node_histories = curr_node_histories;
					context_it->second.explore_index = h_it->second->index;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;

				gather_helper(scope_node_history->scope_history,
							  explore_contexts);

				uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
				context_it->second.node_count++;
				if (select_distribution(generator) == 0) {
					context_it->second.explore_node = node;
					context_it->second.explore_is_branch = false;
					context_it->second.explore_node_histories = curr_node_histories;
					context_it->second.explore_index = h_it->second->index;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				if (branch_node_history->is_branch) {
					uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
					context_it->second.node_count++;
					if (select_distribution(generator) == 0) {
						context_it->second.explore_node = node;
						context_it->second.explore_is_branch = true;
						context_it->second.explore_node_histories = curr_node_histories;
						context_it->second.explore_index = h_it->second->index;
					}
				} else {
					uniform_int_distribution<int> select_distribution(0, context_it->second.node_count);
					context_it->second.node_count++;
					if (select_distribution(generator) == 0) {
						context_it->second.explore_node = node;
						context_it->second.explore_is_branch = false;
						context_it->second.explore_node_histories = curr_node_histories;
						context_it->second.explore_index = h_it->second->index;
					}
				}
			}
			break;
		}
	}
}

void random_node_helper(Scope* scope_context,
						AbstractNode*& node_context,
						bool& is_branch) {
	uniform_int_distribution<int> run_distribution(0, scope_context->run_histories.size()-1);
	int run_index = run_distribution(generator);
	uniform_int_distribution<int> node_history_distribution(0, scope_context->run_histories[run_index].size()-1);
	int node_history_index = node_history_distribution(generator);
	AbstractNode* potential_node_context = scope_context->run_histories[run_index][node_history_index].first;
	bool potential_is_branch = scope_context->run_histories[run_index][node_history_index].second;
	switch (potential_node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)potential_node_context;
			if (noop_node->experiment == NULL) {
				node_context = potential_node_context;
				is_branch = potential_is_branch;
			}
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)potential_node_context;
			if (action_node->experiment == NULL) {
				node_context = potential_node_context;
				is_branch = potential_is_branch;
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)potential_node_context;
			if (scope_node->experiment == NULL) {
				node_context = potential_node_context;
				is_branch = potential_is_branch;
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)potential_node_context;
			if (potential_is_branch) {
				if (branch_node->branch_experiment == NULL) {
					node_context = potential_node_context;
					is_branch = potential_is_branch;
				}
			} else {
				if (branch_node->original_experiment == NULL) {
					node_context = potential_node_context;
					is_branch = potential_is_branch;
				}
			}
		}
		break;
	}
}

void create_experiment(ScopeHistory* scope_history,
					   SolutionWrapper* wrapper) {
	map<Scope*, ExploreContext> explore_contexts;
	gather_helper(scope_history,
				  explore_contexts);

	map<Scope*, ExploreContext>::iterator context_it;
	uniform_int_distribution<int> root_distribution(0, 3);
	if (root_distribution(generator) == 0) {
		context_it = explore_contexts.find(wrapper->solution->starting_scope);
	} else {
		uniform_int_distribution<int> scope_distribution(0, explore_contexts.size()-1);
		context_it = next(explore_contexts.begin(), scope_distribution(generator));
	}
	if (context_it->second.explore_node != NULL) {
		Scope* explore_scope_context = context_it->first;
		AbstractNode* explore_node_context = context_it->second.explore_node;
		bool explore_is_branch = context_it->second.explore_is_branch;

		uniform_int_distribution<int> multi_distribution(0, 1);
		if (multi_distribution(generator) == 0
				&& explore_scope_context->nodes.size() >= MULTI_SCOPE_MIN_NODES
				&& explore_scope_context->run_histories.size() >= RUN_HISTORIES_NUM_SAVE) {
			vector<AbstractNode*> node_contexts;
			vector<bool> is_branch;
			node_contexts.push_back(explore_node_context);
			is_branch.push_back(explore_is_branch);
			for (int try_index = 0; try_index < MULTI_RANDOM_NUM_TRIES; try_index++) {
				AbstractNode* potential_node_context = NULL;
				bool potential_is_branch;
				random_node_helper(explore_scope_context,
								   potential_node_context,
								   potential_is_branch);
				if (potential_node_context != NULL) {
					bool matches_existing = false;
					for (int c_index = 0; c_index < (int)node_contexts.size(); c_index++) {
						if (node_contexts[c_index] == potential_node_context
								&& is_branch[c_index] == potential_is_branch) {
							matches_existing = true;
							break;
						}
					}
					if (!matches_existing) {
						node_contexts.push_back(potential_node_context);
						is_branch.push_back(potential_is_branch);

						if (node_contexts.size() >= MULTI_TARGET_SIZE) {
							break;
						}
					}
				}
			}

			if ((int)node_contexts.size() >= MULTI_TARGET_SIZE) {
				MultiExperiment* new_experiment = new MultiExperiment(
					explore_scope_context,
					node_contexts,
					is_branch,
					wrapper);
				for (int c_index = 0; c_index < (int)node_contexts.size(); c_index++) {
					switch (node_contexts[c_index]->type) {
					case NODE_TYPE_NOOP:
						{
							NoopNode* noop_node = (NoopNode*)node_contexts[c_index];
							noop_node->experiment = new_experiment;
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)node_contexts[c_index];
							action_node->experiment = new_experiment;
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)node_contexts[c_index];
							scope_node->experiment = new_experiment;
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)node_contexts[c_index];
							if (is_branch[c_index]) {
								branch_node->branch_experiment = new_experiment;
							} else {
								branch_node->original_experiment = new_experiment;
							}
						}
						break;
					}
				}
			}
		} else {
			geometric_distribution<int> exit_distribution(0.1);
			int random_index;
			while (true) {
				random_index = context_it->second.explore_index + 1 + exit_distribution(generator);
				if (random_index < (int)context_it->second.explore_node_histories.size()) {
					break;
				}
			}
			AbstractNode* exit_next_node = context_it->second.explore_node_histories[random_index];

			ExploreExperiment* new_experiment = new ExploreExperiment(
				explore_scope_context,
				explore_node_context,
				explore_is_branch,
				exit_next_node,
				wrapper);
			switch (explore_node_context->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)explore_node_context;
					noop_node->experiment = new_experiment;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)explore_node_context;
					action_node->experiment = new_experiment;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)explore_node_context;
					scope_node->experiment = new_experiment;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)explore_node_context;
					if (explore_is_branch) {
						branch_node->branch_experiment = new_experiment;
					} else {
						branch_node->original_experiment = new_experiment;
					}
				}
				break;
			}
		}
	}
}
