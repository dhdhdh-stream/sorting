#include "fold_score_node.h"

using namespace std;

FoldScoreNode::FoldScoreNode(StateNetwork* existing_score_network,
							 int existing_next_node_id,
							 Fold* fold,
							 bool fold_is_pass_through,
							 vector<int> fold_scope_context,
							 vector<int> fold_node_context,
							 int fold_exit_depth,
							 int fold_next_node_id) {
	this->type = NODE_TYPE_FOLD_SCORE

	this->existing_score_network = existing_score_network;
	this->existing_next_node_id = existing_next_node_id;

	this->fold = fold;

	this->fold_is_pass_through = fold_is_pass_through;
	this->fold_scope_context = fold_scope_context;
	this->fold_node_context = fold_node_context;
	this->fold_num_travelled = 0;

	this->fold_exit_depth = fold_exit_depth;
	this->fold_next_node_id = fold_next_node_id;
}

FoldScoreNode::~FoldScoreNode() {
	if (this->existing_score_network != NULL) {
		delete this->existing_score_network;
	}

	if (this->fold != NULL) {
		delete this->fold;
	}
}

void FoldScoreNode::activate(vector<double>& local_state_vals,
							 vector<double>& input_vals,
							 double& predicted_score,
							 double& scale_factor,
							 vector<int>& scope_context,
							 vector<int>& node_context,
							 vector<int>& context_iter,
							 vector<ContextHistory*>& context_histories,
							 int& exit_depth,
							 int& exit_node_id,
							 FoldHistory*& exit_fold_history,
							 RunHelper& run_helper) {
	bool fold_avail = true;
	if (this->fold_num_travelled < 100000) {
		if (randuni() > (double)this->fold_num_travelled/100000) {
			fold_avail = false;
		}
		this->fold_num_travelled++;
	}

	bool matches_context = true;
	if (this->fold_scope_context.size() > scope_context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->fold_scope_context.size(); c_index++) {
			if (this->fold_scope_context[c_index] != scope_context[scope_context.size()-1-c_index]
					|| this->fold_node_context[c_index] != node_context[node_context.size()-1-c_index]) {
				matches_context = false;
				break;
			}
		}
	}
	if (fold_avail && matches_context) {
		FoldHistory* fold_history = new FoldHistory(this->fold);
		this->fold->score_activate(local_state_vals,
								   input_vals,
								   context_iter,
								   context_histories,
								   run_helper,
								   fold_history);
		double fold_score = scale_factor*fold_history->starting_score_update;

		if (this->fold_is_pass_through) {
			history->is_existing = false;

			predicted_score += fold_score;

			exit_depth = this->fold_exit_depth;
			exit_node_id = this->fold_next_node_id;
			exit_fold_history = fold_history;
		} else {
			StateNetworkHistory* existing_network_history = new StateNetworkHistory(this->existing_score_network);
			this->existing_score_network->activate(local_state_vals,
												   input_vals,
												   existing_network_history);
			double existing_score = scale_factor*this->existing_score_network->output->acti_vals[0];

			if (fold_score > existing_score) {
				delete existing_network_history;

				history->is_existing = false;

				predicted_score += fold_score;

				exit_depth = this->fold_exit_depth;
				exit_node_id = this->fold_next_node_id;
				exit_fold_history = fold_history;
			} else {
				delete fold_history;

				history->is_existing = true;
				history->existing_score_network_history = existing_network_history;
				history->existing_score_network_update = this->existing_score_network->output->acti_vals[0];

				predicted_score += existing_score;

				exit_depth = 0;
				exit_node_id = this->existing_next_node_id;
				exit_fold_history = NULL;
			}
		}
	} else {
		StateNetworkHistory* existing_network_history = new StateNetworkHistory(this->existing_score_network);
		this->existing_score_network->activate(local_state_vals,
											   input_vals,
											   existing_network_history);

		history->is_existing = true;
		history->existing_score_network_history = existing_network_history;
		history->existing_score_network_update = this->existing_score_network->output->acti_vals[0];

		predicted_score += scale_factor*this->existing_score_network->output->acti_vals[0];

		exit_depth = 0;
		exit_node_id = this->existing_next_node_id;
		exit_fold_history = NULL;
	}
}

void FoldScoreNode::backprop(vector<double>& local_state_errors,
							 vector<double>& input_errors,
							 double target_val,
							 double& predicted_score,
							 double& scale_factor,
							 RunHelper& run_helper,
							 FoldScoreNodeHistory* history) {
	if (run_helper.explore_phase == EXPLORE_PHASE_FLAT) {
		if (history->is_existing) {
			this->existing_score_network->backprop_errors_with_no_weight_change(
				target_val - predicted_score,
				local_state_errors,
				input_errors,
				history->existing_score_network_history);

			predicted_score -= scale_factor*history->existing_score_network_update;
		}
	} else {
		// run_helper.explore_phase == EXPLORE_PHASE_UPDATE
		if (history->is_existing) {
			this->existing_score_network->backprop_weights_with_no_error_signal(
				target_val - predicted_score,
				0.002,
				history->existing_score_network_history);

			predicted_score -= scale_factor*history->existing_score_network_update;
		}
	}
}

FoldScoreNodeHistory::FoldScoreNodeHistory(FoldScoreNode* node,
										   int scope_index) {
	this->node = node;
	this->scope_index = scope_index;

	this->existing_score_network_history = NULL;
}

FoldScoreNodeHistory::~FoldScoreNodeHistory() {
	if (this->existing_score_network_history != NULL) {
		delete this->existing_score_network_history;
	}
}
