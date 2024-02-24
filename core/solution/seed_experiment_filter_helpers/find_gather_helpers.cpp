#include "seed_experiment_filter.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope_node.h"
#include "seed_experiment.h"
#include "seed_experiment_gather.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void SeedExperimentFilter::find_gather_activate(AbstractNode*& curr_node,
												Problem* problem,
												vector<ContextLayer>& context,
												int& exit_depth,
												AbstractNode*& exit_node,
												RunHelper& run_helper) {
	bool is_target = false;
	SeedExperimentOverallHistory* overall_history = (SeedExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->instance_count++;
	if (!overall_history->has_target) {
		double target_probability;
		if (overall_history->instance_count > this->parent->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->parent->average_instances_per_run - overall_history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		overall_history->has_target = true;

		if (this->parent->sub_state_iter == -1) {
			create_gather_activate(problem,
								   context,
								   run_helper);
		} else {
			if (this->parent->sub_state_iter%2 == 0) {
				curr_node = this->seed_next_node;
			} else {
				for (int s_index = 0; s_index < (int)this->filter_step_types.size(); s_index++) {
					if (this->filter_step_types[s_index] == STEP_TYPE_ACTION) {
						ActionNodeHistory* action_node_history = new ActionNodeHistory(this->filter_actions[s_index]);
						this->filter_actions[s_index]->activate(curr_node,
																problem,
																context,
																exit_depth,
																exit_node,
																run_helper,
																action_node_history);
						delete action_node_history;
					} else if (this->filter_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->filter_existing_scopes[s_index]);
						this->filter_existing_scopes[s_index]->activate(curr_node,
																		problem,
																		context,
																		exit_depth,
																		exit_node,
																		run_helper,
																		scope_node_history);
						delete scope_node_history;
					} else {
						ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->filter_potential_scopes[s_index]);
						this->filter_potential_scopes[s_index]->activate(curr_node,
																		 problem,
																		 context,
																		 exit_depth,
																		 exit_node,
																		 run_helper,
																		 scope_node_history);
						delete scope_node_history;
					}
				}

				if (this->filter_exit_depth == 0) {
					curr_node = this->filter_exit_next_node;
				} else {
					exit_depth = this->filter_exit_depth-1;
					exit_node = this->filter_exit_next_node;
				}
			}
		}
	} else {
		curr_node = this->seed_next_node;
	}
}

void SeedExperimentFilter::create_gather_activate(Problem* problem,
												  vector<ContextLayer>& context,
												  RunHelper& run_helper) {
	vector<Scope*> curr_gather_scope_context;
	vector<AbstractNode*> curr_gather_node_context;
	bool curr_gather_is_branch;
	int curr_gather_exit_depth;
	AbstractNode* curr_gather_exit_node;
	create_gather(curr_gather_scope_context,
				  curr_gather_node_context,
				  curr_gather_is_branch,
				  curr_gather_exit_depth,
				  curr_gather_exit_node,
				  context[context.size() - this->scope_context.size()].scope_history,
				  this);

	vector<int> curr_gather_step_types;
	vector<ActionNode*> curr_gather_actions;
	vector<ScopeNode*> curr_gather_existing_scopes;
	vector<ScopeNode*> curr_gather_potential_scopes;

	uniform_int_distribution<int> uniform_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.5);
	int new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);

	uniform_int_distribution<int> new_scope_distribution(0, 3);
	uniform_int_distribution<int> random_scope_distribution(0, 3);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		ScopeNode* new_scope_node = NULL;
		if (new_scope_distribution(generator) == 0) {
			if (random_scope_distribution(generator) == 0) {
				uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
				Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
				new_scope_node = create_scope(scope,
											  run_helper);
			} else {
				new_scope_node = create_scope(this->scope_context[0],
											  run_helper);
			}
		}
		if (new_scope_node != NULL) {
			curr_gather_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
			curr_gather_actions.push_back(NULL);
			curr_gather_existing_scopes.push_back(NULL);

			curr_gather_potential_scopes.push_back(new_scope_node);
		} else {
			ScopeNode* new_existing_scope_node = reuse_existing();
			if (new_existing_scope_node != NULL) {
				curr_gather_step_types.push_back(STEP_TYPE_EXISTING_SCOPE);
				curr_gather_actions.push_back(NULL);

				curr_gather_existing_scopes.push_back(new_existing_scope_node);

				curr_gather_potential_scopes.push_back(NULL);
			} else {
				curr_gather_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem->random_action();
				curr_gather_actions.push_back(new_action_node);

				curr_gather_existing_scopes.push_back(NULL);
				curr_gather_potential_scopes.push_back(NULL);
			}
		}
	}

	this->parent->curr_gather = new SeedExperimentGather(
		this->parent,
		curr_gather_scope_context,
		curr_gather_node_context,
		curr_gather_is_branch,
		curr_gather_step_types,
		curr_gather_actions,
		curr_gather_existing_scopes,
		curr_gather_potential_scopes,
		curr_gather_exit_depth,
		curr_gather_exit_node);
	if (curr_gather_node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)curr_gather_node_context.back();
		action_node->experiments.insert(action_node->experiments.begin(), this->parent->curr_gather);
	} else if (curr_gather_node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)curr_gather_node_context.back();
		scope_node->experiments.insert(scope_node->experiments.begin(), this->parent->curr_gather);
	} else {
		BranchNode* branch_node = (BranchNode*)curr_gather_node_context.back();
		branch_node->experiments.insert(branch_node->experiments.begin(), this->parent->curr_gather);
		if (curr_gather_is_branch) {
			branch_node->experiment_types.insert(branch_node->experiment_types.begin(), BRANCH_NODE_EXPERIMENT_TYPE_BRANCH);
		} else {
			branch_node->experiment_types.insert(branch_node->experiment_types.begin(), BRANCH_NODE_EXPERIMENT_TYPE_ORIGINAL);
		}
	}

	this->parent->curr_gather_exceeded_limit_count = 0;
	this->parent->curr_gather_is_higher = 0;
	this->parent->curr_gather_score = 0.0;

	// irrelevant what curr_node set to
}
