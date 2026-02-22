#include "experiment.h"

#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

Experiment::Experiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch) {
	this->type = EXPERIMENT_TYPE_EXPERIMENT;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->node_context->experiment = this;

	this->new_network = NULL;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->new_branch_node = NULL;

	this->sum_num_instances = 0;

	this->sum_true = 0.0;
	this->hit_count = 0;

	this->state = EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

Experiment::~Experiment() {
	for (int n_index = 0; n_index < (int)this->existing_networks.size(); n_index++) {
		delete this->existing_networks[n_index];
	}

	if (this->new_network != NULL) {
		delete this->new_network;
	}

	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
	}

	for (int n_index = 0; n_index < (int)this->curr_new_nodes.size(); n_index++) {
		delete this->curr_new_nodes[n_index];
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}

	for (int n_index = 0; n_index < (int)this->best_new_nodes.size(); n_index++) {
		delete this->best_new_nodes[n_index];
	}

	if (this->new_branch_node != NULL) {
		delete this->new_branch_node;
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
