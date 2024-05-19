// #include "new_info_experiment.h"

// #include <iostream>

// #include "action_node.h"
// #include "constants.h"
// #include "network.h"
// #include "problem.h"
// #include "scope.h"
// #include "scope_node.h"

// using namespace std;

// NewInfoExperiment::NewInfoExperiment(Scope* scope_context,
// 									 AbstractNode* node_context,
// 									 bool is_branch,
// 									 AbstractExperiment* parent_experiment) {
// 	this->type = EXPERIMENT_TYPE_NEW_INFO;

// 	this->scope_context = scope_context;
// 	this->node_context = node_context;
// 	this->is_branch = is_branch;

// 	this->parent_experiment = parent_experiment;
// 	if (this->parent_experiment != NULL) {
// 		this->parent_experiment->child_experiments.push_back(this);

// 		AbstractExperiment* curr_experiment = this->parent_experiment;
// 		while (true) {
// 			if (curr_experiment->parent_experiment == NULL) {
// 				break;
// 			} else {
// 				curr_experiment = curr_experiment->parent_experiment;
// 			}
// 		}
// 		this->root_experiment = curr_experiment;
// 	} else {
// 		this->root_experiment = NULL;
// 	}

// 	this->average_remaining_experiments_from_start = 1.0;
// 	/**
// 	 * - start with a 50% chance to bypass
// 	 */
// 	this->average_instances_per_run = 1.0;

// 	this->new_info_subscope = NULL;

// 	this->existing_network = NULL;
// 	this->new_network = NULL;

// 	this->ending_node = NULL;

// 	this->o_target_val_histories.reserve(NUM_DATAPOINTS);

// 	this->state = NEW_INFO_EXPERIMENT_STATE_MEASURE_EXISTING;
// 	this->state_iter = 0;

// 	this->result = EXPERIMENT_RESULT_NA;
// }

// NewInfoExperiment::~NewInfoExperiment() {
// 	if (this->parent_experiment != NULL) {
// 		cout << "inner delete" << endl;
// 	} else {
// 		cout << "outer delete" << endl;
// 	}

// 	if (this->new_info_subscope != NULL) {
// 		delete this->new_info_subscope;
// 	}

// 	if (this->existing_network != NULL) {
// 		delete this->existing_network;
// 	}
// 	if (this->new_network != NULL) {
// 		delete this->new_network;
// 	}

// 	for (int s_index = 0; s_index < (int)this->curr_actions.size(); s_index++) {
// 		if (this->curr_actions[s_index] != NULL) {
// 			delete this->curr_actions[s_index];
// 		}
// 	}

// 	for (int s_index = 0; s_index < (int)this->curr_scopes.size(); s_index++) {
// 		if (this->curr_scopes[s_index] != NULL) {
// 			delete this->curr_scopes[s_index];
// 		}
// 	}

// 	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
// 		if (this->best_actions[s_index] != NULL) {
// 			delete this->best_actions[s_index];
// 		}
// 	}

// 	for (int s_index = 0; s_index < (int)this->best_scopes.size(); s_index++) {
// 		if (this->best_scopes[s_index] != NULL) {
// 			delete this->best_scopes[s_index];
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

// void NewInfoExperiment::decrement(AbstractNode* experiment_node) {
// 	delete this;
// }

// NewInfoExperimentHistory::NewInfoExperimentHistory(NewInfoExperiment* experiment) {
// 	this->experiment = experiment;

// 	this->instance_count = 0;

// 	this->has_target = false;

// 	this->scope_history = NULL;
// }

// NewInfoExperimentHistory::~NewInfoExperimentHistory() {
// 	if (this->scope_history != NULL) {
// 		delete this->scope_history;
// 	}
// }
