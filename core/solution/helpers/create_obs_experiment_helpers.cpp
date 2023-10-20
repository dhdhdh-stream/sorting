#include "helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "flat_network.h"
#include "globals.h"
#include "obs_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"

using namespace std;

/**
 * - practical limit
 *   - increasing increases combinations checked by O(n^4)
 *     - but greatly increases noise, requiring an exponential(?) increase in hidden size (i.e., runtime) to solve
 */
const int NUM_INITIAL_OBS = 10;

void create_obs_experiment_helper(vector<int>& scope_context,
								  vector<int>& node_context,
								  vector<AbstractNode*>& possible_nodes,
								  vector<vector<int>>& possible_scope_contexts,
								  vector<vector<int>>& possible_node_contexts,
								  vector<int>& possible_obs_indexes,
								  ScopeHistory* scope_history);
void create_obs_experiment_branch_experiment_helper(
		vector<int>& scope_context,
		vector<int>& node_context,
		vector<AbstractNode*>& possible_nodes,
		vector<vector<int>>& possible_scope_contexts,
		vector<vector<int>>& possible_node_contexts,
		vector<int>& possible_obs_indexes,
		BranchExperimentHistory* branch_experiment_history) {
	BranchExperiment* branch_experiment = branch_experiment_history->experiment;

	for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
		// leave node_context.back() as -1

		if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			possible_nodes.push_back(branch_experiment->best_actions[s_index]);
			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_obs_indexes.push_back(0);
		} else {
			create_obs_experiment_helper(scope_context,
										 node_context,
										 possible_nodes,
										 possible_scope_contexts,
										 possible_node_contexts,
										 possible_obs_indexes,
										 branch_experiment_history->sequence_histories[s_index]->scope_history);
		}
	}
}

void create_obs_experiment_helper(vector<int>& scope_context,
								  vector<int>& node_context,
								  vector<AbstractNode*>& possible_nodes,
								  vector<vector<int>>& possible_scope_contexts,
								  vector<vector<int>>& possible_node_contexts,
								  vector<int>& possible_obs_indexes,
								  ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				if (action_node->action.move != ACTION_NOOP) {
					node_context.back() = action_node->id;

					possible_nodes.push_back(action_node);
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);
					possible_obs_indexes.push_back(0);

					node_context.back() = -1;
				}

				if (action_node_history->branch_experiment_history != NULL) {
					create_obs_experiment_branch_experiment_helper(
						scope_context,
						node_context,
						possible_nodes,
						possible_scope_contexts,
						possible_node_contexts,
						possible_obs_indexes,
						action_node_history->branch_experiment_history);
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];

				node_context.back() = scope_node_history->node->id;

				uniform_int_distribution<int> distribution(0, 1);
				if (distribution(generator) == 0) {
					create_obs_experiment_helper(scope_context,
												 node_context,
												 possible_nodes,
												 possible_scope_contexts,
												 possible_node_contexts,
												 possible_obs_indexes,
												 scope_node_history->inner_scope_history);
				}

				if (!scope_node_history->is_early_exit) {
					for (map<int, StateStatus>::iterator it = scope_node_history->obs_snapshots.begin();
							it != scope_node_history->obs_snapshots.end(); it++) {
						possible_nodes.push_back(scope_node_history->node);
						possible_scope_contexts.push_back(scope_context);
						possible_node_contexts.push_back(node_context);
						possible_obs_indexes.push_back(it->first);
					}
				}

				node_context.back() = -1;

				if (scope_node_history->branch_experiment_history != NULL) {
					create_obs_experiment_branch_experiment_helper(
						scope_context,
						node_context,
						possible_nodes,
						possible_scope_contexts,
						possible_node_contexts,
						possible_obs_indexes,
						scope_node_history->branch_experiment_history);
				}
			} else {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[i_index][h_index];

				node_context.back() = branch_node_history->node->id;

				possible_nodes.push_back(branch_node_history->node);
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(0);

				node_context.back() = -1;

				if (branch_node_history->branch_experiment_history != NULL) {
					create_obs_experiment_branch_experiment_helper(
						scope_context,
						node_context,
						possible_nodes,
						possible_scope_contexts,
						possible_node_contexts,
						possible_obs_indexes,
						branch_node_history->branch_experiment_history);
				}
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

ObsExperiment* create_obs_experiment(ScopeHistory* scope_history) {
	vector<AbstractNode*> possible_nodes;
	vector<vector<int>> possible_scope_contexts;
	vector<vector<int>> possible_node_contexts;
	vector<int> possible_obs_indexes;

	// TODO: simplify once finished merging sequences into single scope
	if (scope_history->scope->nodes[0]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)scope_history->scope->nodes[0];
		if (action_node->action.move == ACTION_NOOP) {
			possible_nodes.push_back(action_node);
			possible_scope_contexts.push_back(vector<int>{scope_history->scope->id});
			possible_node_contexts.push_back(vector<int>{0});
			possible_obs_indexes.push_back(0);
		}
	}

	while (true) {
		vector<int> scope_context;
		vector<int> node_context;
		create_obs_experiment_helper(scope_context,
									 node_context,
									 possible_nodes,
									 possible_scope_contexts,
									 possible_node_contexts,
									 possible_obs_indexes,
									 scope_history);

		if (possible_nodes.size() > 0) {
			break;
		}
		/**
		 * - may be 0 if only early exit scope node, but don't recurse inwards
		 */
	}

	Scope* parent_scope = scope_history->scope;
	ObsExperiment* obs_experiment = new ObsExperiment(parent_scope);

	int num_obs = min(NUM_INITIAL_OBS, (int)possible_nodes.size());
	for (int o_index = 0; o_index < num_obs; o_index++) {
		uniform_int_distribution<int> distribution(0, (int)possible_nodes.size()-1);
		int rand_obs = distribution(generator);

		obs_experiment->nodes.push_back(possible_nodes[rand_obs]);
		obs_experiment->scope_contexts.push_back(possible_scope_contexts[rand_obs]);
		obs_experiment->node_contexts.push_back(possible_node_contexts[rand_obs]);
		obs_experiment->obs_indexes.push_back(possible_obs_indexes[rand_obs]);

		possible_nodes.erase(possible_nodes.begin() + rand_obs);
		possible_scope_contexts.erase(possible_scope_contexts.begin() + rand_obs);
		possible_node_contexts.erase(possible_node_contexts.begin() + rand_obs);
		possible_obs_indexes.erase(possible_obs_indexes.begin() + rand_obs);
	}

	return obs_experiment;
}
