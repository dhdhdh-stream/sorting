#include "pass_through_experiment.h"

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "start_node.h"

using namespace std;

PassThroughExperiment::PassThroughExperiment(
		Scope* scope_context,
		AbstractNode* node_context,
		bool is_branch,
		SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->new_scope = NULL;

	this->node_context->experiment = this;

	this->num_explores = 0;

	vector<AbstractNode*> possible_exits;

	AbstractNode* starting_node;
	switch (this->node_context->type) {
	case NODE_TYPE_START:
		{
			StartNode* start_node = (StartNode*)this->node_context;
			starting_node = start_node->next_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			starting_node = action_node->next_node;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			starting_node = scope_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				starting_node = branch_node->branch_next_node;
			} else {
				starting_node = branch_node->original_next_node;
			}
		}
		break;
	case NODE_TYPE_BRANCH_END:
		{
			BranchEndNode* branch_end_node = (BranchEndNode*)this->node_context;
			starting_node = branch_end_node->next_node;
		}
		break;
	}

	this->scope_context->random_exit_activate(
		starting_node,
		possible_exits);

	geometric_distribution<int> exit_distribution(0.1);
	int random_index;
	while (true) {
		random_index = exit_distribution(generator);
		if (random_index < (int)possible_exits.size()) {
			break;
		}
	}
	this->exit_next_node = possible_exits[random_index];

	uniform_int_distribution<int> new_scope_distribution(0, 1);
	if (new_scope_distribution(generator) == 0) {
		this->new_scope = create_new_scope(this->node_context->parent);
	}
	if (this->new_scope != NULL) {
		this->step_types.push_back(STEP_TYPE_SCOPE);
		this->actions.push_back(-1);
		this->scopes.push_back(this->new_scope);
	} else {
		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.3);
		/**
		 * - num_steps less than exit length on average to reduce solution size
		 */
		if (random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		vector<int> possible_child_indexes;
		for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
			if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
				possible_child_indexes.push_back(c_index);
			}
		}
		uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool is_scope = false;
			if (possible_child_indexes.size() > 0) {
				if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
					uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
					if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
						is_scope = true;
					}
				} else {
					uniform_int_distribution<int> scope_distribution(0, 1);
					if (scope_distribution(generator) == 0) {
						is_scope = true;
					}
				}
			}
			if (is_scope) {
				this->step_types.push_back(STEP_TYPE_SCOPE);
				this->actions.push_back(-1);

				int child_index = possible_child_indexes[child_index_distribution(generator)];
				this->scopes.push_back(this->node_context->parent->child_scopes[child_index]);
			} else {
				this->step_types.push_back(STEP_TYPE_ACTION);

				this->actions.push_back(-1);

				this->scopes.push_back(NULL);
			}
		}
	}

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = scope_context;

			this->new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = scope_context;
			new_scope_node->id = scope_context->node_counter;
			scope_context->node_counter++;

			new_scope_node->scope = this->scopes[s_index];

			this->new_nodes.push_back(new_scope_node);
		}
	}

	this->total_count = 0;
	this->total_sum_scores = 0.0;

	this->sum_scores = 0.0;

	this->state = PASS_THROUGH_EXPERIMENT_STATE_C1;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

PassThroughExperiment::~PassThroughExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		delete this->new_nodes[n_index];
	}

	for (int h_index = 0; h_index < (int)this->new_scope_histories.size(); h_index++) {
		delete this->new_scope_histories[h_index];
	}
}

PassThroughExperimentHistory::PassThroughExperimentHistory(PassThroughExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;

	this->num_instances = 0;
}

PassThroughExperimentState::PassThroughExperimentState(PassThroughExperiment* experiment) {
	this->experiment = experiment;
}
