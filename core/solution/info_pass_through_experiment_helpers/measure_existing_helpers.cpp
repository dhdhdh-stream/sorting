#include "info_pass_through_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope_node.h"
#include "new_action_tracker.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void InfoPassThroughExperiment::measure_existing_activate(
		InfoPassThroughExperimentHistory* history) {
	history->instance_count++;
}

void InfoPassThroughExperiment::measure_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	InfoPassThroughExperimentHistory* history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories.back();

	this->o_target_val_histories.push_back(target_val);

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (!run_helper.exceeded_limit) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;
		}

		if (run_helper.num_actions > solution->max_num_actions) {
			solution->max_num_actions = run_helper.num_actions;
		}
	}

	if (run_helper.new_action_history != NULL) {
		for (int n_index = 0; n_index < (int)run_helper.new_action_history->existing_path_taken.size(); n_index++) {
			NewActionNodeTracker* node_tracker = solution->new_action_tracker->node_trackers[
				run_helper.new_action_history->existing_path_taken[n_index]];
			node_tracker->existing_score += target_val;
			node_tracker->existing_count++;
		}
		for (int n_index = 0; n_index < (int)run_helper.new_action_history->new_path_taken.size(); n_index++) {
			NewActionNodeTracker* node_tracker = solution->new_action_tracker->node_trackers[
				run_helper.new_action_history->new_path_taken[n_index]];
			node_tracker->new_score += target_val;
			node_tracker->new_count++;
		}
	}

	if ((int)this->o_target_val_histories.size() >= NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / NUM_DATAPOINTS;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_standard_deviation = sqrt(sum_score_variance / NUM_DATAPOINTS);
		if (this->existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->o_target_val_histories.clear();

		vector<AbstractNode*> possible_exits;

		if (this->node_context->type == NODE_TYPE_ACTION
				&& ((ActionNode*)this->node_context)->next_node == NULL) {
			possible_exits.push_back(NULL);
		}

		AbstractNode* starting_node;
		switch (this->node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				starting_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_INFO_SCOPE:
			{
				InfoScopeNode* info_scope_node = (InfoScopeNode*)this->node_context;
				starting_node = info_scope_node->next_node;
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNode* info_branch_node = (InfoBranchNode*)this->node_context;
				if (this->is_branch) {
					starting_node = info_branch_node->branch_next_node;
				} else {
					starting_node = info_branch_node->original_next_node;
				}
			}
			break;
		}

		this->scope_context->random_exit_activate(
			starting_node,
			possible_exits);

		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
		int random_index = distribution(generator);
		this->exit_next_node = possible_exits[random_index];

		this->info_scope = get_existing_info_scope();
		uniform_int_distribution<int> negate_distribution(0, 1);
		this->is_negate = negate_distribution(generator) == 0;

		int new_num_steps;
		uniform_int_distribution<int> uniform_distribution(0, 1);
		geometric_distribution<int> geometric_distribution(0.5);
		if (random_index == 0) {
			new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
		} else {
			new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
		}

		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			InfoScopeNode* new_scope_node = create_existing_info_scope_node();
			if (new_scope_node != NULL) {
				this->step_types.push_back(STEP_TYPE_SCOPE);
				this->actions.push_back(NULL);

				this->scopes.push_back(new_scope_node);
			} else {
				this->step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->actions.push_back(new_action_node);

				this->scopes.push_back(NULL);
			}
		}

		this->new_score = 0.0;

		this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
