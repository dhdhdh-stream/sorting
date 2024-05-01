#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_experiment.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void create_experiment_helper(vector<Scope*>& possible_scope_contexts,
							  vector<AbstractNode*>& possible_node_contexts,
							  vector<bool>& possible_is_branch,
							  ScopeHistory* scope_history) {
	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
			{
				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(false);
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;

				if (solution->state == SOLUTION_STATE_TRAVERSE
						&& solution->state_iter >= 5) {
					create_experiment_helper(possible_scope_contexts,
											 possible_node_contexts,
											 possible_is_branch,
											 scope_node_history->scope_history);
				}

				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(false);
			}

			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

				possible_scope_contexts.push_back(scope_history->scope);
				possible_node_contexts.push_back(it->first);
				possible_is_branch.push_back(branch_node_history->is_branch);
			}

			break;
		}
	}
}

void create_experiment(ScopeHistory* root_history) {
	vector<Scope*> possible_scope_contexts;
	vector<AbstractNode*> possible_node_contexts;
	vector<bool> possible_is_branch;

	create_experiment_helper(possible_scope_contexts,
							 possible_node_contexts,
							 possible_is_branch,
							 root_history);

	uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
	int rand_index = possible_distribution(generator);

	uniform_int_distribution<int> experiment_type_distribution(0, 1);
	if (experiment_type_distribution(generator) == 0) {
		BranchExperiment* new_experiment = new BranchExperiment(
			possible_scope_contexts[rand_index],
			possible_node_contexts[rand_index],
			possible_is_branch[rand_index],
			NULL);

		possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
	} else {
		PassThroughExperiment* new_experiment = new PassThroughExperiment(
			possible_scope_contexts[rand_index],
			possible_node_contexts[rand_index],
			possible_is_branch[rand_index],
			NULL);

		possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
	}
}

void create_eval_experiment(ScopeHistory* root_history) {
	vector<Scope*> possible_scope_contexts;
	vector<AbstractNode*> possible_node_contexts;
	vector<bool> possible_is_branch;

	create_experiment_helper(possible_scope_contexts,
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
