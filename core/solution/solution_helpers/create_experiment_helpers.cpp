#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_pass_through_experiment.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "new_action_experiment.h"
#include "new_info_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void info_create_experiment_helper(InfoScope* info_scope,
								   vector<InfoScope*>& possible_info_scope_contexts,
								   vector<Scope*>& possible_scope_contexts,
								   vector<AbstractNode*>& possible_node_contexts,
								   vector<bool>& possible_is_branch,
								   ScopeHistory* scope_history) {
	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
			{
				possible_info_scope_contexts.push_back(info_scope);
				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(false);
			}

			break;
		case NODE_TYPE_INFO_SCOPE:
			{
				InfoScopeNode* info_scope_node = (InfoScopeNode*)it->first;
				InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;

				if (info_scope_node->scope->state == INFO_SCOPE_STATE_NA
						&& info_scope_node_history->scope_history != NULL) {
					info_create_experiment_helper(info_scope_node->scope,
												  possible_info_scope_contexts,
												  possible_scope_contexts,
												  possible_node_contexts,
												  possible_is_branch,
												  info_scope_node_history->scope_history);
				}

				possible_info_scope_contexts.push_back(info_scope);
				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(false);
			}

			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)it->first;
				InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;

				if (info_branch_node->scope->state == INFO_SCOPE_STATE_NA
						&& info_branch_node_history->scope_history != NULL) {
					info_create_experiment_helper(info_branch_node->scope,
												  possible_info_scope_contexts,
												  possible_scope_contexts,
												  possible_node_contexts,
												  possible_is_branch,
												  info_branch_node_history->scope_history);
				}

				possible_info_scope_contexts.push_back(info_scope);
				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(info_branch_node_history->is_branch);
			}

			break;
		}
	}
}

void create_experiment_helper(vector<InfoScope*>& possible_info_scope_contexts,
							  vector<Scope*>& possible_scope_contexts,
							  vector<AbstractNode*>& possible_node_contexts,
							  vector<bool>& possible_is_branch,
							  ScopeHistory* scope_history) {
	uniform_int_distribution<int> include_distribution(0, (int)scope_history->scope->nodes.size()-1);

	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
			{
				if (include_distribution(generator) == 0) {
					possible_info_scope_contexts.push_back(NULL);
					possible_scope_contexts.push_back(scope_history->scope);
					possible_node_contexts.push_back(it->first);
					possible_is_branch.push_back(false);
				}
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				create_experiment_helper(possible_info_scope_contexts,
										 possible_scope_contexts,
										 possible_node_contexts,
										 possible_is_branch,
										 scope_node_history->scope_history);

				if (include_distribution(generator) == 0) {
					possible_info_scope_contexts.push_back(NULL);
					possible_scope_contexts.push_back(scope_history->scope);
					possible_node_contexts.push_back(it->first);
					possible_is_branch.push_back(false);
				}
			}

			break;
		case NODE_TYPE_BRANCH:
			{
				if (include_distribution(generator) == 0) {
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

					possible_info_scope_contexts.push_back(NULL);
					possible_scope_contexts.push_back(scope_history->scope);
					possible_node_contexts.push_back(it->first);
					possible_is_branch.push_back(branch_node_history->is_branch);
				}
			}

			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)it->first;
				InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;

				if (info_branch_node->scope->state == INFO_SCOPE_STATE_NA
						&& info_branch_node_history->scope_history != NULL) {
					uniform_int_distribution<int> info_inner_distribution(0, 4);
					if (info_inner_distribution(generator) == 0) {
						info_create_experiment_helper(info_branch_node->scope,
													  possible_info_scope_contexts,
													  possible_scope_contexts,
													  possible_node_contexts,
													  possible_is_branch,
													  info_branch_node_history->scope_history);
					}
				}

				if (include_distribution(generator) == 0) {
					possible_info_scope_contexts.push_back(NULL);
					possible_scope_contexts.push_back(scope_history->scope);
					possible_node_contexts.push_back(it->first);
					possible_is_branch.push_back(info_branch_node_history->is_branch);
				}
			}

			break;
		}
	}
}

void create_experiment(ScopeHistory* root_history) {
	vector<InfoScope*> possible_info_scope_contexts;
	vector<Scope*> possible_scope_contexts;
	vector<AbstractNode*> possible_node_contexts;
	vector<bool> possible_is_branch;

	create_experiment_helper(possible_info_scope_contexts,
							 possible_scope_contexts,
							 possible_node_contexts,
							 possible_is_branch,
							 root_history);

	if (possible_scope_contexts.size() > 0) {
		uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
		int rand_index = possible_distribution(generator);

		if (possible_info_scope_contexts[rand_index] == NULL) {
			uniform_int_distribution<int> expensive_distribution(0, 9);
			if (expensive_distribution(generator) == 0) {
				uniform_int_distribution<int> type_distribution(0, 1);
				switch (type_distribution(generator)) {
				case 0:
					{
						// NewInfoExperiment* new_experiment = new NewInfoExperiment(
						// 	possible_scope_contexts[rand_index],
						// 	possible_node_contexts[rand_index],
						// 	possible_is_branch[rand_index],
						// 	NULL);

						// possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
					}
					break;
				case 1:
					{
						BranchExperiment* new_experiment = new BranchExperiment(
							possible_scope_contexts[rand_index],
							possible_node_contexts[rand_index],
							possible_is_branch[rand_index],
							NULL);

						possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
					}
					break;
				}
			} else {
				uniform_int_distribution<int> pass_through_distribution(0, 3);
				if (pass_through_distribution(generator) != 0) {
					#if defined(MDEBUG) && MDEBUG
					if (possible_scope_contexts[rand_index]->nodes.size() > 10) {
					#else
					if (possible_scope_contexts[rand_index]->nodes.size() > 20) {
					#endif /* MDEBUG */
						NewActionExperiment* new_action_experiment = new NewActionExperiment(
							possible_scope_contexts[rand_index],
							possible_node_contexts[rand_index],
							possible_is_branch[rand_index]);

						if (new_action_experiment->result == EXPERIMENT_RESULT_FAIL) {
							delete new_action_experiment;
						} else {
							possible_node_contexts[rand_index]->experiments.push_back(new_action_experiment);
						}
					}
				} else {
					// PassThroughExperiment* new_experiment = new PassThroughExperiment(
					// 	possible_scope_contexts[rand_index],
					// 	possible_node_contexts[rand_index],
					// 	possible_is_branch[rand_index],
					// 	NULL);

					// possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
				}
			}
		} else {
			// InfoPassThroughExperiment* new_experiment = new InfoPassThroughExperiment(
			// 	possible_info_scope_contexts[rand_index],
			// 	possible_scope_contexts[rand_index],
			// 	possible_node_contexts[rand_index],
			// 	possible_is_branch[rand_index]);

			// possible_info_scope_contexts[rand_index]->experiment = new_experiment;
			// possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
		}
	}
}
