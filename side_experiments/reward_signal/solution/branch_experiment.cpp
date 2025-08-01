#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "factor.h"
#include "globals.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch,
								   SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->curr_new_scope = NULL;
	this->curr_scope_history = NULL;
	this->best_new_scope = NULL;
	this->best_scope_history = NULL;

	this->new_network = NULL;

	this->new_branch_node = NULL;

	vector<ScopeHistory*> scope_histories;
	vector<double> target_val_histories;
	for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
		if (hit_helper(wrapper->solution->existing_scope_histories[h_index],
					   this->scope_context,
					   this->node_context,
					   this->is_branch)) {
			fetch_histories_helper(wrapper->solution->existing_scope_histories[h_index],
								   wrapper->solution->existing_target_val_histories[h_index],
								   this->scope_context,
								   this->node_context,
								   this->is_branch,
								   scope_histories,
								   target_val_histories);

			this->existing_scores.push_back(wrapper->solution->existing_target_val_histories[h_index]);
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

		this->existing_average_score = average_score;
		this->existing_inputs = factor_inputs;
		this->existing_input_averages = factor_input_averages;
		this->existing_input_standard_deviations = factor_input_standard_deviations;
		this->existing_weights = factor_weights;

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

		this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;

		this->result = EXPERIMENT_RESULT_NA;
	} else {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}

BranchExperiment::~BranchExperiment() {
	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
	}

	if (this->curr_scope_history != NULL) {
		delete this->curr_scope_history;
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}

	if (this->best_scope_history != NULL) {
		delete this->best_scope_history;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
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

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void BranchExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

BranchExperimentOverallHistory::BranchExperimentOverallHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

BranchExperimentInstanceHistory::BranchExperimentInstanceHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->signal_scope_node = NULL;
	this->signal_needed_from = NULL;
}

BranchExperimentState::BranchExperimentState(BranchExperiment* experiment) {
	this->experiment = experiment;
}
