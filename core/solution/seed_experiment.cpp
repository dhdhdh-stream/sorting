#include "seed_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

SeedExperiment::SeedExperiment(vector<Scope*> scope_context,
							   vector<AbstractNode*> node_context,
							   bool is_branch) {
	this->type = EXPERIMENT_TYPE_SEED;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->existing_network = NULL;

	this->best_exit_node = NULL;

	this->curr_filter = NULL;

	this->curr_gather = NULL;

	this->filter_step_index = 0;

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);
	this->i_scope_histories.reserve(solution->curr_num_datapoints);
	this->i_target_val_histories.reserve(solution->curr_num_datapoints);

	this->state = SEED_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->train_gather_iter = 0;

	this->best_surprise = 0.0;

	this->combined_score = 0.0;

	this->result = EXPERIMENT_RESULT_NA;
}

SeedExperiment::~SeedExperiment() {
	if (this->existing_network != NULL) {
		delete this->existing_network;
	}

	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
		if (this->best_actions[s_index] != NULL) {
			delete this->best_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_existing_scopes.size(); s_index++) {
		if (this->best_existing_scopes[s_index] != NULL) {
			delete this->best_existing_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_potential_scopes.size(); s_index++) {
		if (this->best_potential_scopes[s_index] != NULL) {
			delete this->best_potential_scopes[s_index]->scope;
			delete this->best_potential_scopes[s_index];
		}
	}

	if (this->best_exit_node != NULL) {
		delete this->best_exit_node;
	}

	// let nodes delete filters/gathers

	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
		delete this->i_scope_histories[h_index];
	}
}

SeedExperimentOverallHistory::SeedExperimentOverallHistory(
		SeedExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
}
