// #include "info_pass_through_experiment.h"

// #include <iostream>

// #include "action_node.h"
// #include "constants.h"
// #include "info_scope_node.h"
// #include "network.h"
// #include "problem.h"
// #include "scope.h"

// using namespace std;

// InfoPassThroughExperiment::InfoPassThroughExperiment(
// 		InfoScope* info_scope_context,
// 		Scope* scope_context,
// 		AbstractNode* node_context,
// 		bool is_branch) {
// 	this->type = EXPERIMENT_TYPE_INFO_PASS_THROUGH;

// 	this->info_scope_context = info_scope_context;

// 	this->scope_context = scope_context;
// 	this->node_context = node_context;
// 	this->is_branch = is_branch;

// 	this->average_remaining_experiments_from_start = 1.0;
// 	/**
// 	 * - start with a 50% chance to bypass
// 	 */
// 	this->average_instances_per_run = 1.0;

// 	this->negative_network = NULL;
// 	this->positive_network = NULL;

// 	this->ending_node = NULL;

// 	this->o_target_val_histories.reserve(NUM_DATAPOINTS);

// 	this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING;
// 	this->state_iter = 0;

// 	this->result = EXPERIMENT_RESULT_NA;
// }

// InfoPassThroughExperiment::~InfoPassThroughExperiment() {
// 	cout << "outer delete" << endl;

// 	if (this->negative_network != NULL) {
// 		delete this->negative_network;
// 	}
// 	if (this->positive_network != NULL) {
// 		delete this->positive_network;
// 	}

// 	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
// 		if (this->actions[s_index] != NULL) {
// 			delete this->actions[s_index];
// 		}
// 	}

// 	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
// 		if (this->scopes[s_index] != NULL) {
// 			delete this->scopes[s_index];
// 		}
// 	}

// 	if (this->ending_node != NULL) {
// 		delete this->ending_node;
// 	}

// 	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
// 		delete this->i_scope_histories[h_index];
// 	}

// 	#if defined(MDEBUG) && MDEBUG
// 	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
// 		delete this->verify_problems[p_index];
// 	}
// 	#endif /* MDEBUG */
// }

// void InfoPassThroughExperiment::decrement(AbstractNode* experiment_node) {
// 	delete this;
// }

// InfoPassThroughExperimentHistory::InfoPassThroughExperimentHistory(
// 		InfoPassThroughExperiment* experiment) {
// 	this->experiment = experiment;

// 	this->instance_count = 0;
// }
