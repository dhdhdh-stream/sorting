#include "chase_experiment.h"

#include "abstract_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

ChaseExperiment::ChaseExperiment(Scope* scope_context,
								 AbstractNode* node_context,
								 bool is_branch,
								 SolutionWrapper* wrapper) {
	this->type = EXPERIMENT_TYPE_CHASE;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->node_context->experiment = this;

	// uniform_int_distribution<int> tunnel_distribution(0, wrapper->solution->tunnels.size()-1);
	// this->tunnel = wrapper->solution->tunnels[tunnel_distribution(generator)];
	this->tunnel = wrapper->solution->tunnels[3];

	this->existing_true_network = NULL;
	this->existing_signal_network = NULL;
	this->new_signal_network = NULL;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->sum_num_instances = 0;

	this->total_count = 0;

	this->sum_true = 0.0;
	this->sum_signal = 0.0;

	this->state = CHASE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

ChaseExperiment::~ChaseExperiment() {
	if (this->existing_true_network != NULL) {
		delete this->existing_true_network;
	}

	if (this->existing_signal_network != NULL) {
		delete this->existing_signal_network;
	}

	if (this->new_signal_network != NULL) {
		delete this->new_signal_network;
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

ChaseExperimentHistory::ChaseExperimentHistory(ChaseExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

ChaseExperimentState::ChaseExperimentState(ChaseExperiment* experiment) {
	this->experiment = experiment;
}
