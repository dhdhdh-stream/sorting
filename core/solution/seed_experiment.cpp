#include "seed_experiment.h"

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

	for (int a_index = 0; a_index < (int)this->step_types.size(); a_index++) {
		if (this->step_types[a_index] == STEP_TYPE_ACTION) {
			if (this->state == SEED_EXPERIMENT_STATE_EXPLORE) {
				delete this->actions[a_index];
			}
		} else if (this->step_types[a_index] == STEP_TYPE_EXISTING_SCOPE) {
			if (this->state == SEED_EXPERIMENT_STATE_EXPLORE) {
				delete this->existing_scopes[a_index];
			}
		} else {
			delete this->potential_scopes[a_index]->scope;
			/**
			 * - should be safe due to the order in which nodes are being deleted in local scope
			 */
			if (this->state == SEED_EXPERIMENT_STATE_EXPLORE) {
				delete this->potential_scopes[a_index];
			}
		}
	}

	/**
	 * - let nodes delete filters/gathers
	 */
}
