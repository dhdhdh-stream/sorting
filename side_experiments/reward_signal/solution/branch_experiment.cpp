#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
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

BranchExperiment::BranchExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch,
								   Input reward_signal,
								   SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	if (reward_signal.scope_context.size() != 0) {
		this->reward_signal = reward_signal;

		ObsNode* obs_node = (ObsNode*)this->reward_signal.scope_context.back()
			->nodes[this->reward_signal.node_context.back()];
		Factor* factor = obs_node->factors[this->reward_signal.factor_index];
		this->reward_signal_average = factor->average;
		this->reward_signal_standard_deviation = factor->standard_deviation;
	}

	this->curr_scope_history = NULL;
	this->best_scope_history = NULL;

	this->new_network = NULL;

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
			scope_histories.push_back(scope_history);

			if (reward_signal.scope_context.size() != 0) {
				double val;
				bool is_on;
				fetch_input_helper(scope_history,
								   this->reward_signal,
								   0,
								   val,
								   is_on);
				if (is_on) {
					double target_val = (val - this->reward_signal_average)
						/ this->reward_signal_standard_deviation;
					target_val_histories.push_back(target_val);
				} else {
					target_val_histories.push_back(-1.0);
				}
			} else {
				target_val_histories.push_back(this->scope_context->existing_target_val_histories[h_index]);
			}
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
									 factor_weights,
									 this,
									 wrapper);

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
	if (this->curr_scope_history != NULL) {
		delete this->curr_scope_history;
	}

	if (this->best_scope_history != NULL) {
		delete this->best_scope_history;
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}

	for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
		delete this->scope_histories[h_index];
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

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->has_explore = false;
}

BranchExperimentState::BranchExperimentState(BranchExperiment* experiment) {
	this->experiment = experiment;
}
