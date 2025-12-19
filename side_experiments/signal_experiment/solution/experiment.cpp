#include "experiment.h"

#include "abstract_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"

using namespace std;

Experiment::Experiment(Scope* scope_context,
					   AbstractNode* node_context,
					   bool is_branch,
					   SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_EXPERIMENT;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->node_context->experiment = this;

	this->existing_network = NULL;
	this->new_network = NULL;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->sum_num_instances = 0;

	this->total_count = 0;

	this->sum_scores = 0.0;

	// temp
	this->sum_existing_true = 0.0;
	this->sum_existing_signal = 0.0;

	this->state = EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

Experiment::~Experiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
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

	for (int h_index = 0; h_index < (int)this->new_scope_histories.size(); h_index++) {
		delete this->new_scope_histories[h_index];
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
