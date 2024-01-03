#include "retrain_loop_experiment.h"

#include <iostream>

#include "globals.h"
#include "solution_helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void RetrainLoopExperiment::finalize() {
	cout << "retrain_loop success" << endl << endl;

	Scope* parent_scope = solution->scopes[this->scope_node->loop_scope_context[0]];
	parent_scope->temp_states.insert(parent_scope->temp_states.end(),
		this->new_states.begin(), this->new_states.end());
	parent_scope->temp_state_nodes.insert(parent_scope->temp_state_nodes.end(),
		this->new_state_nodes.begin(), this->new_state_nodes.end());
	parent_scope->temp_state_scope_contexts.insert(parent_scope->temp_state_scope_contexts.end(),
		this->new_state_scope_contexts.begin(), this->new_state_scope_contexts.end());
	parent_scope->temp_state_node_contexts.insert(parent_scope->temp_state_node_contexts.end(),
		this->new_state_node_contexts.begin(), this->new_state_node_contexts.end());
	parent_scope->temp_state_obs_indexes.insert(parent_scope->temp_state_obs_indexes.end(),
		this->new_state_obs_indexes.begin(), this->new_state_obs_indexes.end());
	parent_scope->temp_state_new_local_indexes.insert(parent_scope->temp_state_new_local_indexes.end(),
		this->new_states.size(), -1);

	this->new_states.clear();
	this->new_state_nodes.clear();
	this->new_state_scope_contexts.clear();
	this->new_state_node_contexts.clear();
	this->new_state_obs_indexes.clear();

	this->scope_node->decision_standard_deviation = this->existing_standard_deviation;

	this->scope_node->loop_state_is_local.clear();
	this->scope_node->loop_state_indexes.clear();
	this->scope_node->loop_continue_weights.clear();
	this->scope_node->loop_halt_weights.clear();

	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	finalize_loop_scope_node_states(this->scope_node,
									this->scope_node->loop_scope_context,
									this->scope_node->loop_node_context,
									this->halt_input_state_weights,
									this->halt_local_state_weights,
									this->halt_temp_state_weights,
									this->continue_input_state_weights,
									this->continue_local_state_weights,
									this->continue_temp_state_weights,
									input_scope_depths_mappings,
									output_scope_depths_mappings);

	this->scope_node->max_iters++;

	#if defined(MDEBUG) && MDEBUG
	this->scope_node->verify_key = this;
	this->scope_node->verify_continue_scores = this->verify_continue_scores;
	this->scope_node->verify_halt_scores = this->verify_halt_scores;
	this->scope_node->verify_factors = this->verify_factors;
	#endif /* MDEBUG */

	this->state = RETRAIN_LOOP_EXPERIMENT_STATE_SUCCESS;
}
