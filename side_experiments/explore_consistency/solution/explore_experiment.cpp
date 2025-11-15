#include "explore_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_instance.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "start_node.h"

using namespace std;

ExploreExperiment::ExploreExperiment(Scope* scope_context,
									 AbstractNode* node_context,
									 bool is_branch,
									 SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_EXPLORE;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->node_context->experiment = this;

	this->curr_explore_instance = new ExploreInstance();
	this->curr_explore_instance->experiment = this;

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
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)this->node_context;
			starting_node = obs_node->next_node;
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
	this->curr_explore_instance->exit_next_node = possible_exits[random_index];

	uniform_int_distribution<int> new_scope_distribution(0, 1);
	if (new_scope_distribution(generator) == 0) {
		this->curr_explore_instance->new_scope = create_new_scope(this->node_context->parent);
	}
	if (this->curr_explore_instance->new_scope != NULL) {
		this->curr_explore_instance->step_types.push_back(STEP_TYPE_SCOPE);
		this->curr_explore_instance->actions.push_back(-1);
		this->curr_explore_instance->scopes.push_back(this->curr_explore_instance->new_scope);
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
				this->curr_explore_instance->step_types.push_back(STEP_TYPE_SCOPE);
				this->curr_explore_instance->actions.push_back(-1);

				int child_index = possible_child_indexes[child_index_distribution(generator)];
				this->curr_explore_instance->scopes.push_back(this->node_context->parent->child_scopes[child_index]);
			} else {
				this->curr_explore_instance->step_types.push_back(STEP_TYPE_ACTION);

				this->curr_explore_instance->actions.push_back(-1);

				this->curr_explore_instance->scopes.push_back(NULL);
			}
		}
	}

	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

void ExploreExperiment::clean() {
	this->node_context->experiment = NULL;
}

ExploreExperimentHistory::ExploreExperimentHistory(ExploreExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;

	this->num_instances = 0;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
