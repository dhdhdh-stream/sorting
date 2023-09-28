#include "solution.h"

using namespace std;

/**
 * - practical limit
 *   - increasing increases combinations checked by O(n^4)
 *     - but greatly increases noise, requiring an exponential(?) increase in hidden size (i.e., runtime) to solve
 */
const int NUM_INITIAL_OBS = 10;

void Solution::create_obs_experiment_branch_experiment_helper(
		vector<int>& scope_context,
		vector<int>& node_context,
		vector<AbstractNode*>& possible_nodes,
		vector<vector<int>>& possible_scope_contexts,
		vector<vector<int>>& possible_node_contexts,
		vector<int>& possible_obs_indexes,
		BranchExperimentHistory* branch_experiment_history) {
	BranchExperiment* branch_experiment = branch_experiment_history->experiment;

	for (int s_index = 0; s_index < (int)branch_experiment->step_types.size(); s_index++) {
		// simply leave node_context.back() as -1;

		if (branch_experiment->step_types[s_index] == STEP_TYPE_ACTION) {
			possible_nodes.push_back(branch_experiment->actions[s_index]);
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

void Solution::create_obs_experiment_helper(vector<int>& scope_context,
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
			AbstractNode* node = scope_history->node_histories[i_index][h_index]->node;
			node_context.back() = node->id;

			if (node->type == NODE_TYPE_ACTION) {
				possible_nodes.push_back(node);
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(0);
			} else if (node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];

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

				for (map<int, StateStatus>::iterator it = scope_node_history->obs_snapshots.begin();
						it != scope_node_history->obs_snapshots.end(); it++) {
					possible_nodes.push_back(node);
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);
					possible_obs_indexes.push_back(it->first);
				}
			} else {
				// node->type == NODE_TYPE_BRANCH
				possible_nodes.push_back(node);
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(0);
			}

			node_context.back() = -1;

			if (scope_history->node_histories[i_index][h_index]->branch_experiment_history != NULL) {
				create_obs_experiment_branch_experiment_helper(
					scope_context,
					node_context,
					possible_nodes,
					possible_scope_contexts,
					possible_node_contexts,
					possible_obs_indexes,
					scope_history->node_histories[i_index][h_index]->branch_experiment_history);
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

ObsExperiment* Solution::create_obs_experiment(ScopeHistory* scope_history) {
	vector<AbstractNode*> possible_nodes;
	vector<vector<int>> possible_scope_contexts;
	vector<vector<int>> possible_node_contexts;
	vector<int> possible_obs_indexes;

	vector<int> scope_context;
	vector<int> node_context;
	create_obs_experiment_helper(scope_context,
								 node_context,
								 possible_nodes,
								 possible_scope_contexts,
								 possible_node_contexts,
								 possible_obs_indexes,
								 scope_history);

	ObsExperiment* obs_experiment = new ObsExperiment();

	int num_obs = min(NUM_INITIAL_OBS, possible_nodes.size());
	for (int o_index = 0; o_index < num_obs; o_index++) {
		uniform_int_distribution<int> distribution(0, possible_nodes.size()-1);
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

	obs_experiment->flat_network = new FlatNetwork(num_obs);

	return obs_experiment;
}
