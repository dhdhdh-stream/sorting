#include "seed_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "problem.h"
#include "scope_node.h"
#include "seed_experiment_filter.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void SeedExperiment::create_filter() {
	geometric_distribution<int> step_index_distribution(0.5);
	this->curr_filter_step_index = this->filter_step_index + 1 + step_index_distribution(generator);
	if (this->curr_filter_step_index > (int)this->best_step_types.size()) {
		this->curr_filter_step_index = (int)this->best_step_types.size();
	}

	vector<Scope*> filter_scope_context = this->scope_context;
	vector<AbstractNode*> filter_node_context = this->node_context;
	if (this->best_step_types[this->curr_filter_step_index-1] == STEP_TYPE_ACTION) {
		filter_node_context.back() = this->best_actions[this->curr_filter_step_index-1];
	} else if (this->best_step_types[this->curr_filter_step_index-1] == STEP_TYPE_EXISTING_SCOPE) {
		filter_node_context.back() = this->best_existing_scopes[this->curr_filter_step_index-1];
	} else {
		filter_node_context.back() = this->best_potential_scopes[this->curr_filter_step_index-1];
	}

	AbstractNode* seed_next_node;
	if (this->curr_filter_step_index == (int)this->best_step_types.size()) {
		if (this->best_exit_depth == 0) {
			seed_next_node = this->best_exit_next_node;
		} else {
			seed_next_node = this->best_exit_node;
		}
	} else {
		if (this->best_step_types[this->curr_filter_step_index] == STEP_TYPE_ACTION) {
			seed_next_node = this->best_actions[this->curr_filter_step_index];
		} else if (this->best_step_types[this->curr_filter_step_index] == STEP_TYPE_EXISTING_SCOPE) {
			seed_next_node = this->best_existing_scopes[this->curr_filter_step_index];
		} else {
			seed_next_node = this->best_potential_scopes[this->curr_filter_step_index];
		}
	}

	vector<pair<int,AbstractNode*>> possible_exits;
	gather_possible_exits(possible_exits,
						  this->scope_context,
						  this->node_context,
						  this->is_branch);

	uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
	int random_index = distribution(generator);
	double filter_exit_depth = possible_exits[random_index].first;
	AbstractNode* filter_exit_next_node = possible_exits[random_index].second;

	uniform_int_distribution<int> uniform_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.5);
	int new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);

	vector<int> filter_step_types;
	vector<ActionNode*> filter_actions;
	vector<ScopeNode*> filter_existing_scopes;
	vector<ScopeNode*> filter_potential_scopes;

	uniform_int_distribution<int> new_scope_distribution(0, 3);
	uniform_int_distribution<int> random_scope_distribution(0, 3);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		ScopeNode* new_scope_node = NULL;
		if (new_scope_distribution(generator) == 0) {
			if (random_scope_distribution(generator) == 0) {
				uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
				Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
				RunHelper run_helper;
				new_scope_node = create_scope(scope,
											  run_helper);
			} else {
				RunHelper run_helper;
				new_scope_node = create_scope(this->scope_context[0],
											  run_helper);
			}
		}
		if (new_scope_node != NULL) {
			filter_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
			filter_actions.push_back(NULL);
			filter_existing_scopes.push_back(NULL);

			filter_potential_scopes.push_back(new_scope_node);
		} else {
			ScopeNode* new_existing_scope_node = reuse_existing();
			if (new_existing_scope_node != NULL) {
				filter_step_types.push_back(STEP_TYPE_EXISTING_SCOPE);
				filter_actions.push_back(NULL);

				filter_existing_scopes.push_back(new_existing_scope_node);

				filter_potential_scopes.push_back(NULL);
			} else {
				filter_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				filter_actions.push_back(new_action_node);

				filter_existing_scopes.push_back(NULL);
				filter_potential_scopes.push_back(NULL);
			}
		}
	}

	this->curr_filter = new SeedExperimentFilter(this,
												 filter_scope_context,
												 filter_node_context,
												 false,
												 seed_next_node,
												 filter_step_types,
												 filter_actions,
												 filter_existing_scopes,
												 filter_potential_scopes,
												 filter_exit_depth,
												 filter_exit_next_node);
	if (filter_node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)filter_node_context.back();
		action_node->experiments.push_back(this->curr_filter);
	} else {
		ScopeNode* scope_node = (ScopeNode*)filter_node_context.back();
		scope_node->experiments.push_back(this->curr_filter);
	}

	this->curr_filter_score = 0.0;
}
