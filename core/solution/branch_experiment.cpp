#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

BranchExperiment::BranchExperiment(vector<Scope*> scope_context,
								   vector<AbstractNode*> node_context,
								   bool is_branch,
								   int throw_id,
								   PassThroughExperiment* parent_experiment,
								   bool skip_explore) {
	this->type = EXPERIMENT_TYPE_BRANCH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->throw_id = throw_id;

	this->parent_experiment = parent_experiment;
	if (this->parent_experiment != NULL) {
		this->parent_experiment->child_experiments.push_back(this);

		PassThroughExperiment* curr_experiment = this->parent_experiment;
		while (true) {
			if (curr_experiment->parent_experiment == NULL) {
				break;
			} else {
				curr_experiment = curr_experiment->parent_experiment;
			}
		}
		this->root_experiment = curr_experiment;
	} else {
		this->root_experiment = NULL;
	}

	this->skip_explore = skip_explore;

	if (this->skip_explore) {
		this->average_remaining_experiments_from_start = 0.0;
	} else {
		this->average_remaining_experiments_from_start = 1.0;
	}
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);
	this->i_scope_histories.reserve(solution->curr_num_datapoints);
	this->i_target_val_histories.reserve(solution->curr_num_datapoints);

	this->state = BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->branch_node = NULL;
	this->exit_node = NULL;

	this->combined_score = 0.0;
	this->original_count = 0;
	this->branch_count = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

BranchExperiment::~BranchExperiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
	}
	if (this->new_network != NULL) {
		delete this->new_network;
	}

	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		if (this->actions[s_index] != NULL) {
			delete this->actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->existing_scopes.size(); s_index++) {
		if (this->existing_scopes[s_index] != NULL) {
			delete this->existing_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->potential_scopes.size(); s_index++) {
		if (this->potential_scopes[s_index] != NULL) {
			delete this->potential_scopes[s_index]->scope;
			delete this->potential_scopes[s_index];
		}
	}

	if (this->branch_node != NULL) {
		delete this->branch_node;
	}

	if (this->exit_node != NULL) {
		delete this->exit_node;
	}

	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
		delete this->i_scope_histories[h_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
}
