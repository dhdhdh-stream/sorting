#include "retrain_loop_experiment.h"

#include "solution_helpers.h"
#include "scope_node.h"

using namespace std;

void RetrainLoopExperiment::finalize() {
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
