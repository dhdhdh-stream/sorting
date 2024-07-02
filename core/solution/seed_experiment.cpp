#include "seed_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

SeedExperiment::SeedExperiment(AbstractScope* scope_context,
							   AbstractNode* node_context,
							   bool is_branch,
							   int score_type) {
	this->type = EXPERIMENT_TYPE_SEED;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->score_type = score_type;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->new_network = NULL;

	this->scope_histories.reserve(NUM_DATAPOINTS);
	this->target_val_histories.reserve(NUM_DATAPOINTS);

	this->state = SEED_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

SeedExperiment::~SeedExperiment() {
	if (this->new_network != NULL) {
		delete this->new_network;
	}

	for (int s_index = 0; s_index < (int)this->curr_seed_actions.size(); s_index++) {
		if (this->curr_seed_actions[s_index] != NULL) {
			delete this->curr_seed_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_seed_scopes.size(); s_index++) {
		if (this->curr_seed_scopes[s_index] != NULL) {
			delete this->curr_seed_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_seed_actions.size(); s_index++) {
		if (this->best_seed_actions[s_index] != NULL) {
			delete this->best_seed_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_seed_scopes.size(); s_index++) {
		if (this->best_seed_scopes[s_index] != NULL) {
			delete this->best_seed_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_back_actions.size(); s_index++) {
		if (this->curr_back_actions[s_index] != NULL) {
			delete this->curr_back_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->curr_back_scopes.size(); s_index++) {
		if (this->curr_back_scopes[s_index] != NULL) {
			delete this->curr_back_scopes[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_back_actions.size(); s_index++) {
		if (this->best_back_actions[s_index] != NULL) {
			delete this->best_back_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_back_scopes.size(); s_index++) {
		if (this->best_back_scopes[s_index] != NULL) {
			delete this->best_back_scopes[s_index];
		}
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

void SeedExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

SeedExperimentHistory::SeedExperimentHistory(SeedExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
}
