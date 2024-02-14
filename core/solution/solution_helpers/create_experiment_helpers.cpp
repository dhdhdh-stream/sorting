#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "globals.h"
#include "outer_experiment.h"
#include "pass_through_experiment.h"
#include "retrain_branch_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void create_experiment_helper(vector<Scope*>& scope_context,
							  vector<AbstractNode*>& node_context,
							  vector<vector<Scope*>>& possible_scope_contexts,
							  vector<vector<AbstractNode*>>& possible_node_contexts,
							  ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			if (h_index == 0 || action_node->action.move != ACTION_NOOP) {
				if (action_node->experiment == NULL) {
					node_context.back() = action_node;

					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = NULL;
				}
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			create_experiment_helper(scope_context,
									 node_context,
									 possible_scope_contexts,
									 possible_node_contexts,
									 scope_node_history->scope_history);

			if (scope_node->experiment == NULL) {
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
			}

			node_context.back() = NULL;
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;
			BranchNode* branch_node = (BranchNode*)branch_node_history->node;
			if (!branch_node->is_pass_through) {
				if (branch_node->experiment == NULL) {
					node_context.back() = branch_node;

					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = NULL;
				}
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void create_experiment(ScopeHistory* root_history) {
	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	create_experiment_helper(scope_context,
							 node_context,
							 possible_scope_contexts,
							 possible_node_contexts,
							 root_history);

	uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
	int rand_index = possible_distribution(generator);

	uniform_int_distribution<int> outer_distribution(0, max(9, (int)possible_scope_contexts[rand_index].size()));
	if (outer_distribution(generator) == 0) {
		solution->outer_experiment = new OuterExperiment();
	} else {
		if (possible_node_contexts[rand_index].back()->type == NODE_TYPE_ACTION) {
			uniform_int_distribution<int> next_distribution(0, 1);
			int context_size = 1;
			// while (true) {
			// 	if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
			// 		context_size++;
			// 	} else {
			// 		break;
			// 	}
			// }
			/**
			 * - minimize context to generalize/maximize impact
			 */

			uniform_int_distribution<int> pass_through_distribution(0, 1);
			if (pass_through_distribution(generator) == 0) {
				PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
					vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
					vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

				ActionNode* action_node = (ActionNode*)possible_node_contexts[rand_index].back();
				action_node->experiment = new_pass_through_experiment;
			} else {
				BranchExperiment* new_branch_experiment = new BranchExperiment(
					vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
					vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

				ActionNode* action_node = (ActionNode*)possible_node_contexts[rand_index].back();
				action_node->experiment = new_branch_experiment;
			}
		} else if (possible_node_contexts[rand_index].back()->type == NODE_TYPE_SCOPE) {
			uniform_int_distribution<int> next_distribution(0, 1);
			int context_size = 1;
			// while (true) {
			// 	if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
			// 		context_size++;
			// 	} else {
			// 		break;
			// 	}
			// }

			uniform_int_distribution<int> pass_through_distribution(0, 1);
			if (pass_through_distribution(generator) == 0) {
				PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
					vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
					vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

				ScopeNode* scope_node = (ScopeNode*)possible_node_contexts[rand_index].back();
				scope_node->experiment = new_pass_through_experiment;
			} else {
				BranchExperiment* new_branch_experiment = new BranchExperiment(
					vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
					vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

				ScopeNode* scope_node = (ScopeNode*)possible_node_contexts[rand_index].back();
				scope_node->experiment = new_branch_experiment;
			}
		} else {
			uniform_int_distribution<int> retrain_distribution(0, 1);
			if (retrain_distribution(generator) == 0) {
				BranchNode* branch_node = (BranchNode*)possible_node_contexts[rand_index].back();
				RetrainBranchExperiment* new_retrain_branch_experiment = new RetrainBranchExperiment(branch_node);
				branch_node->experiment = new_retrain_branch_experiment;
			} else {
				uniform_int_distribution<int> next_distribution(0, 1);
				int context_size = 1;
				// while (true) {
				// 	if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
				// 		context_size++;
				// 	} else {
				// 		break;
				// 	}
				// }

				uniform_int_distribution<int> pass_through_distribution(0, 1);
				uniform_int_distribution<int> is_branch_distribution(0, 1);
				if (pass_through_distribution(generator) == 0) {
					PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
						vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
						vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

					BranchNode* branch_node = (BranchNode*)possible_node_contexts[rand_index].back();
					branch_node->experiment = new_pass_through_experiment;
					branch_node->experiment_is_branch = is_branch_distribution(generator) == 0;
				} else {
					BranchExperiment* new_branch_experiment = new BranchExperiment(
						vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
						vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

					BranchNode* branch_node = (BranchNode*)possible_node_contexts[rand_index].back();
					branch_node->experiment = new_branch_experiment;
					branch_node->experiment_is_branch = is_branch_distribution(generator) == 0;
				}
			}
		}
	}
}
