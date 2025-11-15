#include "experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "explore_experiment.h"
#include "explore_instance.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

Experiment::Experiment(ExploreInstance* explore_instance) {
	this->type = EXPERIMENT_TYPE_EXPERIMENT;

	this->explore_experiment = explore_instance->experiment;

	this->scope_context = explore_instance->experiment->scope_context;
	this->node_context = explore_instance->experiment->node_context;
	this->is_branch = explore_instance->experiment->is_branch;

	this->new_scope = explore_instance->new_scope;
	explore_instance->new_scope = NULL;
	this->step_types = explore_instance->step_types;
	this->actions = explore_instance->actions;
	this->scopes = explore_instance->scopes;
	this->exit_next_node = explore_instance->exit_next_node;

	this->new_val_network = NULL;

	this->node_context->experiment = this;

	uniform_int_distribution<int> until_distribution(1, this->explore_experiment->average_instances_per_run);
	this->num_instances_until_target = until_distribution(generator);

	this->state = EXPERIMENT_STATE_TRAIN_NEW;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

Experiment::~Experiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	if (this->new_val_network != NULL) {
		delete this->new_val_network;
	}

	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		delete this->new_nodes[n_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

ExperimentHistory::ExperimentHistory(Experiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

ExperimentState::ExperimentState(Experiment* experiment) {
	this->experiment = experiment;
}
