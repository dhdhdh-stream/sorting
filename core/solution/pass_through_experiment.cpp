#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

PassThroughExperiment::PassThroughExperiment(AbstractScope* scope_context,
											 AbstractNode* node_context,
											 bool is_branch,
											 int score_type,
											 AbstractExperiment* parent_experiment) {
	this->type = EXPERIMENT_TYPE_PASS_THROUGH;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->score_type = score_type;

	this->parent_experiment = parent_experiment;
	if (this->parent_experiment != NULL) {
		this->parent_experiment->child_experiments.push_back(this);

		AbstractExperiment* curr_experiment = this->parent_experiment;
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

	this->average_remaining_experiments_from_start = 1.0;
	this->average_instances_per_run = 1.0;

	this->info_branch_node = NULL;

	this->target_val_histories.reserve(NUM_DATAPOINTS);

	this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->curr_score = 0.0;

	this->best_score = numeric_limits<double>::lowest();

	this->result = EXPERIMENT_RESULT_NA;
}

PassThroughExperiment::~PassThroughExperiment() {
	for (int s_index = 0; s_index < (int)this->curr_actions.size(); s_index++) {
		if (this->curr_actions[s_index] != NULL) {
			delete this->curr_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_scopes.size(); s_index++) {
		if (this->curr_scopes[s_index] != NULL) {
			delete this->curr_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
		if (this->best_actions[s_index] != NULL) {
			delete this->best_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_scopes.size(); s_index++) {
		if (this->best_scopes[s_index] != NULL) {
			delete this->best_scopes[s_index];
		}
	}

	if (this->info_branch_node != NULL) {
		delete this->info_branch_node;
	}
}

void PassThroughExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

PassThroughExperimentHistory::PassThroughExperimentHistory(
		PassThroughExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
}
