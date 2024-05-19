// #include "pass_through_experiment.h"

// #include <iostream>

// #include "action_node.h"
// #include "branch_node.h"
// #include "constants.h"
// #include "globals.h"
// #include "info_branch_node.h"
// #include "problem.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution.h"
// #include "solution_helpers.h"

// using namespace std;

// void PassThroughExperiment::measure_existing_activate(
// 		PassThroughExperimentHistory* history) {
// 	history->instance_count++;
// }

// void PassThroughExperiment::measure_existing_backprop(
// 		double target_val,
// 		RunHelper& run_helper) {
// 	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

// 	this->o_target_val_histories.push_back(target_val);

// 	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

// 	if (this->parent_experiment == NULL) {
// 		if (!run_helper.exceeded_limit) {
// 			if (run_helper.max_depth > solution->max_depth) {
// 				solution->max_depth = run_helper.max_depth;
// 			}

// 			if (run_helper.num_actions > solution->max_num_actions) {
// 				solution->max_num_actions = run_helper.num_actions;
// 			}
// 		}
// 	}

// 	if ((int)this->o_target_val_histories.size() >= NUM_DATAPOINTS) {
// 		double sum_scores = 0.0;
// 		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
// 			sum_scores += this->o_target_val_histories[d_index];
// 		}
// 		this->existing_average_score = sum_scores / NUM_DATAPOINTS;

// 		this->o_target_val_histories.clear();

// 		vector<AbstractNode*> possible_exits;

// 		if (this->node_context->type == NODE_TYPE_ACTION
// 				&& ((ActionNode*)this->node_context)->next_node == NULL) {
// 			possible_exits.push_back(NULL);
// 		}

// 		AbstractNode* starting_node;
// 		switch (this->node_context->type) {
// 		case NODE_TYPE_ACTION:
// 			{
// 				ActionNode* action_node = (ActionNode*)this->node_context;
// 				starting_node = action_node->next_node;
// 			}
// 			break;
// 		case NODE_TYPE_SCOPE:
// 			{
// 				ScopeNode* scope_node = (ScopeNode*)this->node_context;
// 				starting_node = scope_node->next_node;
// 			}
// 			break;
// 		case NODE_TYPE_BRANCH:
// 			{
// 				BranchNode* branch_node = (BranchNode*)this->node_context;
// 				if (this->is_branch) {
// 					starting_node = branch_node->branch_next_node;
// 				} else {
// 					starting_node = branch_node->original_next_node;
// 				}
// 			}
// 			break;
// 		case NODE_TYPE_INFO_BRANCH:
// 			{
// 				InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
// 				if (this->is_branch) {
// 					starting_node = info_branch_node->branch_next_node;
// 				} else {
// 					starting_node = info_branch_node->original_next_node;
// 				}
// 			}
// 			break;
// 		}

// 		this->scope_context->random_exit_activate(
// 			starting_node,
// 			possible_exits);

// 		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
// 		int random_index = distribution(generator);
// 		this->curr_exit_next_node = possible_exits[random_index];

// 		this->curr_info_scope = get_existing_info_scope();
// 		uniform_int_distribution<int> negate_distribution(0, 1);
// 		this->curr_is_negate = negate_distribution(generator) == 0;

// 		int new_num_steps;
// 		uniform_int_distribution<int> uniform_distribution(0, 1);
// 		geometric_distribution<int> geometric_distribution(0.5);
// 		if (random_index == 0) {
// 			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
// 		} else {
// 			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
// 		}

// 		uniform_int_distribution<int> default_distribution(0, 3);
// 		for (int s_index = 0; s_index < new_num_steps; s_index++) {
// 			bool default_to_action = true;
// 			if (default_distribution(generator) != 0) {
// 				ScopeNode* new_scope_node = create_existing();
// 				if (new_scope_node != NULL) {
// 					this->curr_step_types.push_back(STEP_TYPE_SCOPE);
// 					this->curr_actions.push_back(NULL);

// 					this->curr_scopes.push_back(new_scope_node);

// 					default_to_action = false;
// 				}
// 			}

// 			if (default_to_action) {
// 				this->curr_step_types.push_back(STEP_TYPE_ACTION);

// 				ActionNode* new_action_node = new ActionNode();
// 				new_action_node->action = problem_type->random_action();
// 				this->curr_actions.push_back(new_action_node);

// 				this->curr_scopes.push_back(NULL);
// 			}
// 		}

// 		this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE;
// 		this->state_iter = 0;
// 		this->sub_state_iter = 0;
// 	}
// }
