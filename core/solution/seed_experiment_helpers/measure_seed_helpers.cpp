#include "seed_experiment.h"

using namespace std;

void SeedExperiment::measure_seed_activate() {
	bool is_target = false;
	if (!overall_history->has_target) {
		double target_probability;
		if (overall_history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - overall_history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		overall_history->has_target = true;

		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0]->type == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else if (this->best_step_types[0]->type == STEP_TYPE_EXISTING_SCOPE) {
				curr_node = this->best_existing_scopes[0];
			} else {
				curr_node = this->best_potential_scopes[0];
			}
		}
	}
}

void SeedExperiment::measure_seed_backprop(double target_val,
										   SeedExperimentOverallHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		int num_higher = 0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			if (this->o_target_val_histories[d_index] > this->existing_average_score + this->existing_score_standard_deviation) {
				num_higher++;
			}
		}
		this->new_higher_ratio = num_higher / solution->curr_num_datapoints;

		this->o_target_val_histories.clear();

		AbstractNode* curr_filter_next_node;
		if (this->node_context.back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context.back();
			curr_filter_next_node = action_node->next_node;
		} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
			curr_filter_next_node = scope_node->next_node;
		} else {
			BranchNode* branch_node = (BranchNode*)this->node_context.back();
			if (this->is_branch) {
				curr_filter_next_node = branch_node->branch_next_node;
			} else {
				curr_filter_next_node = branch_node->original_next_node;
			}
		}
		this->curr_filter = new SeedExperimentFilter(this,
													 this->scope_context,
													 this->node_context,
													 this->is_branch,
													 vector<int>(),
													 vector<ActionNode*>(),
													 vector<ScopeNode*>(),
													 vector<ScopeNode*>(),
													 0,
													 curr_filter_next_node);
		if (this->node_context.back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context.back();
			action_node->experiments.push_back(this->curr_filter);
		} else if (this->node_context.back()->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->node_context.back();
			scope_node->experiments.push_back(this->curr_filter);
		} else {
			BranchNode* branch_node = (BranchNode*)this->node_context.back();
			branch_node->experiments.push_back(this->curr_filter);
			branch_node->experiment_is_branch.push_back(this->is_branch);
		}

		this->i_scope_histories.reserve(solution->curr_num_datapoints);
		this->i_target_val_histories.reserve(solution->curr_num_datapoints);

		this->state = SEED_EXPERIMENT_STATE_TRAIN_FILTER;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
