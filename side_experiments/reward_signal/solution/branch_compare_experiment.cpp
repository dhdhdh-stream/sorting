#include "branch_compare_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

BranchCompareExperiment::BranchCompareExperiment(
		Scope* scope_context,
		AbstractNode* node_context,
		bool is_branch) {
	this->type = EXPERIMENT_TYPE_BRANCH_COMPARE;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->new_true_network = NULL;
	this->new_signal_network = NULL;

	this->new_branch_node = NULL;

	vector<ScopeHistory*> scope_histories;
	vector<double> target_val_histories;
	for (int h_index = 0; h_index < (int)this->scope_context->existing_scope_histories.size(); h_index++) {
		ScopeHistory* scope_history = this->scope_context->existing_scope_histories[h_index];

		bool has_match = false;

		map<int, AbstractNodeHistory*>::iterator match_it = scope_history->node_histories.find(this->node_context->id);
		if (match_it != scope_history->node_histories.end()) {
			if (this->node_context->type == NODE_TYPE_BRANCH) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)match_it->second;
				if (branch_node_history->is_branch == this->is_branch) {
					has_match = true;
				}
			} else {
				has_match = true;
			}
		}

		if (has_match) {
			ScopeHistory* cleaned_scope_history = new ScopeHistory(scope_history, match_it->second->index);
			scope_histories.push_back(cleaned_scope_history);

			double reward_signal = calc_reward_signal(scope_history);
			target_val_histories.push_back(reward_signal);
		}
	}

	double average_score;
	vector<Input> factor_inputs;
	vector<double> factor_input_averages;
	vector<double> factor_input_standard_deviations;
	vector<double> factor_weights;
	bool is_success = train_existing(scope_histories,
									 target_val_histories,
									 average_score,
									 factor_inputs,
									 factor_input_averages,
									 factor_input_standard_deviations,
									 factor_weights);

	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		delete scope_histories[h_index];
	}

	if (is_success) {
		this->node_context->experiment = this;

		this->existing_signal_average_score = average_score;
		this->existing_signal_inputs = factor_inputs;
		this->existing_signal_input_averages = factor_input_averages;
		this->existing_signal_input_standard_deviations = factor_input_standard_deviations;
		this->existing_signal_weights = factor_weights;

		this->best_surprise = 0.0;

		switch (this->node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				this->average_instances_per_run = action_node->average_instances_per_run;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				this->average_instances_per_run = scope_node->average_instances_per_run;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					this->average_instances_per_run = branch_node->branch_average_instances_per_run;
				} else {
					this->average_instances_per_run = branch_node->original_average_instances_per_run;
				}
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)this->node_context;
				this->average_instances_per_run = obs_node->average_instances_per_run;
			}
			break;
		}

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = BRANCH_COMPARE_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;

		this->result = EXPERIMENT_RESULT_NA;
	} else {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}

BranchCompareExperiment::~BranchCompareExperiment() {
	if (this->new_true_network != NULL) {
		delete this->new_true_network;
	}

	if (this->new_signal_network != NULL) {
		delete this->new_signal_network;
	}

	if (this->new_branch_node != NULL) {
		delete this->new_branch_node;
	}
	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		delete this->new_nodes[n_index];
	}

	for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
		delete this->scope_histories[h_index];
	}

	for (int h_index = 0; h_index < (int)this->new_scope_histories.size(); h_index++) {
		delete this->new_scope_histories[h_index];
	}
}

void BranchCompareExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

BranchCompareExperimentHistory::BranchCompareExperimentHistory(BranchCompareExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

BranchCompareExperimentState::BranchCompareExperimentState(BranchCompareExperiment* experiment) {
	this->experiment = experiment;
}
