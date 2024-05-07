#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_experiment.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_pass_through_experiment.h"
#include "info_scope_node.h"
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

				if (info_scope_node_history->scope_history != NULL) {
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

				if (info_branch_node_history->scope_history != NULL) {
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
	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
			{
				possible_info_scope_contexts.push_back(NULL);
				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(false);
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				if (solution->state == SOLUTION_STATE_TRAVERSE
						&& solution->current->nodes.size() > 10) {
					create_experiment_helper(possible_info_scope_contexts,
											 possible_scope_contexts,
											 possible_node_contexts,
											 possible_is_branch,
											 scope_node_history->scope_history);
				}

				possible_info_scope_contexts.push_back(NULL);
				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(false);
			}

			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

				possible_info_scope_contexts.push_back(NULL);
				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(branch_node_history->is_branch);
			}

			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)it->first;
				InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;

				if (info_branch_node_history->scope_history != NULL) {
					if (solution->state == SOLUTION_STATE_TRAVERSE
							&& solution->current->nodes.size() > 10) {
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
				}

				possible_info_scope_contexts.push_back(NULL);
				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(info_branch_node_history->is_branch);
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

	uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
	int rand_index = possible_distribution(generator);

	if (possible_info_scope_contexts[rand_index] == NULL) {
		uniform_int_distribution<int> branch_distribution(0, 3);
		if (branch_distribution(generator) == 0) {
			uniform_int_distribution<int> info_distribution(0, 1);
			if (info_distribution(generator) == 0) {
				NewInfoExperiment* new_experiment = new NewInfoExperiment(
					possible_scope_contexts[rand_index],
					possible_node_contexts[rand_index],
					possible_is_branch[rand_index],
					NULL);

				possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
			} else {
				BranchExperiment* new_experiment = new BranchExperiment(
					possible_scope_contexts[rand_index],
					possible_node_contexts[rand_index],
					possible_is_branch[rand_index],
					NULL);

				possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
			}
		} else {
			PassThroughExperiment* new_experiment = new PassThroughExperiment(
				possible_scope_contexts[rand_index],
				possible_node_contexts[rand_index],
				possible_is_branch[rand_index],
				NULL);

			possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
		}
	} else {
		InfoPassThroughExperiment* new_experiment = new InfoPassThroughExperiment(
			possible_info_scope_contexts[rand_index],
			possible_scope_contexts[rand_index],
			possible_node_contexts[rand_index],
			possible_is_branch[rand_index]);

		possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
	}
}

void create_eval_experiment(ScopeHistory* root_history) {
	vector<InfoScope*> possible_info_scope_contexts;
	vector<Scope*> possible_scope_contexts;
	vector<AbstractNode*> possible_node_contexts;
	vector<bool> possible_is_branch;

	create_experiment_helper(possible_info_scope_contexts,
							 possible_scope_contexts,
							 possible_node_contexts,
							 possible_is_branch,
							 root_history);

	uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
	int rand_index = possible_distribution(generator);

	EvalExperiment* new_experiment = new EvalExperiment(
		possible_node_contexts[rand_index],
		possible_is_branch[rand_index]);

	possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
}
