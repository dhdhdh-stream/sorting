#include "branch_node.h"

using namespace std;



void BranchNode::activate(vector<ForwardContextLayer>& context,
						  RunHelper& run_helper,
						  BranchNodeHistory* history) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		// special case first scope context
		if (this->branch_scope_context[0] != context.back().scope_id) {
			matches_context = false;
		} else {
			for (int c_index = 1; c_index < (int)this->branch_scope_context.size(); c_index++) {
				if (this->branch_scope_context[c_index] != context[scope_context.size()-1-c_index].scope_id
						|| this->branch_node_context[c_index] != context[node_context.size()-1-c_index].nodes) {
					matches_context = false;
					break;
				}
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			history.is_branch = true;

			// no need to activate score networks
		} else {
			history->state_vals_snapshot = *(context.back().state_vals);

			ScoreNetworkHistory* branch_network_history = new ScoreNetworkHistory(this->branch_score_network);
			this->branch_score_network->activate(history->state_vals_snapshot,
												 branch_network_history);
			double branch_score = run_helper.scale_factor*this->branch_score_network->output->acti_vals[0];

			ScoreNetworkHistory* original_network_history = new ScoreNetworkHistory(this->original_score_network);
			this->original_score_network->activate(history->state_vals_snapshot,
												   original_network_history);
			double original_score = run_helper.scale_factor*this->original_score_network->output->acti_vals[0];

			if (branch_score > original_score) {
				history.is_branch = true;

				delete original_network_history;
				history->score_network_history = branch_network_history;
				history->score_network_update = this->branch_score_network->output->acti_vals[0];
			} else {
				history.is_branch = false;

				delete branch_network_history;
				history->score_network_history = original_network_history;
				history->score_network_update = this->original_score_network->output->acti_vals[0];
			}
		}
	} else {
		history.is_branch = false;

		// no need to activate score networks
	}
}

void BranchNode::backprop(vector<BackwardContextLayer>& context,
						  RunHelper& run_helper,
						  BranchNodeHistory* history) {
	/**
	 * - don't factor in branch score networks in scale_factor_error
	 *   - right path should be 0.0
	 *   - wrong path will be inaccurate
	 */

	if (history->score_network_history != NULL) {
		double branch_predicted_score = run_helper.predicted_score + run_helper.scale_factor*history->score_network_update;

		double predicted_score_error = run_helper.target_val - branch_predicted_score;

		ScoreNetwork* score_network = history->score_network_history->network;
		if (run_helper.explore_phase == EXPLORE_PHASE_UPDATE
				|| run_helper.explore_phase == EXPLORE_PHASE_CLEANUP) {
			score_network->backprop_weights_with_no_error_signal(
				predicted_score_error,
				0.002,
				history->state_vals_snapshot,
				history->score_network_history);
		} else if (run_helper.explore_phase == EXPLORE_PHASE_EXPERIMENT
				|| run_helper.explore_phase == EXPLORE_PHASE_NEW_CLASSES) {
			score_network->backprop_errors_with_no_weight_change(
				predicted_score_error,
				*(context.back().state_errors),
				history->state_vals_snapshot,
				history->score_network_history);
		}
	}
}


