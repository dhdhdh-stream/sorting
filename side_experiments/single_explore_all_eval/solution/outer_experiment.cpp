#include "outer_experiment.h"

#include "abstract_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

OuterExperiment::OuterExperiment(Scope* scope_context,
								 AbstractNode* node_context,
								 bool is_branch,
								 AbstractNode* exit_next_node) {
	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	this->node_context->experiment = this;

	this->existing_true_network = NULL;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->sum_num_instances = 0;

	this->state = OUTER_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
}

OuterExperiment::~OuterExperiment() {
	if (this->existing_true_network != NULL) {
		delete this->existing_true_network;
	}

	for (int n_index = 0; n_index < (int)this->new_networks.size(); n_index++) {
		delete this->new_networks[n_index];
	}

	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}
}

OuterExperimentHistory::OuterExperimentHistory(OuterExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;

	this->hit_branch = false;
}

OuterExperimentState::OuterExperimentState(OuterExperiment* experiment) {
	this->experiment = experiment;
}
