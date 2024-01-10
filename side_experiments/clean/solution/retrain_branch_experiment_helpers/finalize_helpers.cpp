#include "retrain_branch_experiment.h"

#include "branch_node.h"
#include "solution_helpers.h"

using namespace std;

void RetrainBranchExperiment::finalize() {
	this->branch_node->original_score_mod = this->original_average_score;
	this->branch_node->branch_score_mod = this->branch_average_score;

	this->branch_node->decision_standard_deviation = this->existing_standard_deviation;

	this->branch_node->decision_state_is_local.clear();
	this->branch_node->decision_state_indexes.clear();
	this->branch_node->decision_original_weights.clear();
	this->branch_node->decision_branch_weights.clear();

	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	finalize_branch_node_states(this->branch_node,
								this->original_input_state_weights,
								this->original_local_state_weights,
								this->original_temp_state_weights,
								this->branch_input_state_weights,
								this->branch_local_state_weights,
								this->branch_temp_state_weights,
								input_scope_depths_mappings,
								output_scope_depths_mappings);

	#if defined(MDEBUG) && MDEBUG
	this->branch_node->verify_key = this;
	this->branch_node->verify_original_scores = this->verify_original_scores;
	this->branch_node->verify_branch_scores = this->verify_branch_scores;
	this->branch_node->verify_factors = this->verify_factors;
	#endif /* MDEBUG */

	this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_SUCCESS;
}
