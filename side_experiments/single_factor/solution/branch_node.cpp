#include "branch_node.h"

using namespace std;



void BranchNode::activate(vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  bool& is_branch,
						  vector<vector<AbstractNodeHistory*>>& node_histories) {
	bool matches_context = true;
	if (this->scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
			if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope_id
					|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node_id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			is_branch = true;

			// don't modify running_average_score/running_average_misguess
		} else {
			vector<vector<double>*> state_vals(this->context_indexes.size()+1);
			state_vals[0] = context[0].state_vals;
			for (int c_index = 0; c_index < (int)this->context_indexes.size(); c_index++) {
				state_vals[1+c_index] = context[context.size()-this->context_indexes.size()+c_index].state_vals;
			}

			if (run_helper.phase == UPDATE_PHASE_NONE
					&& this->remeasure_counter > solution->remeasure_target) {
				run_helper.phase = UPDATE_PHASE_REMEASURE;
				run_helper.remeasure_type = REMEASURE_TYPE_BRANCH;
				RemeasureBranchNodeHistory* remeasure_branch_node_history = new RemeasureBranchNodeHistory(this);
				run_helper.remeasure_branch_node_history = remeasure_branch_node_history;

				remeasure_branch_node_history->running_average_score_snapshot = run_helper.running_average_score;
				remeasure_branch_node_history->running_average_misguess_snapshot = run_helper.running_average_misguess;
				remeasure_branch_node_history->scale_factor_snapshot = run_helper.scale_factor;

				if (rand()%2 == 0) {
					is_branch = true;

					ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->branch_score_network);
					this->branch_score_network->activate(state_vals,
														 score_network_history);
					remeasure_branch_node_history->score_network_history = score_network_history;
					remeasure_branch_node_history->score_network_output = this->branch_score_network->output->acti_vals[0];

					ScoreNetworkHistory* misguess_network_history = new ScoreNetworkHistory(this->branch_misguess_network);
					this->branch_misguess_network->activate(state_vals,
															misguess_network_history);
					remeasure_branch_node_history->misguess_network_history = misguess_network_history;
					remeasure_branch_node_history->misguess_network_output = this->branch_misguess_network->output->acti_vals[0];
				} else {
					is_branch = false;

					ScoreNetworkHistory* score_network_history = new ScoreNetworkHistory(this->original_score_network);
					this->original_score_network->activate(state_vals,
														   score_network_history);
					remeasure_branch_node_history->score_network_history = score_network_history;
					remeasure_branch_node_history->score_network_output = this->original_score_network->output->acti_vals[0];

					ScoreNetworkHistory* misguess_network_history = new ScoreNetworkHistory(this->original_misguess_network);
					this->original_misguess_network->activate(state_vals,
															  misguess_network_history);
					remeasure_branch_node_history->misguess_network_history = misguess_network_history;
					remeasure_branch_node_history->misguess_network_output = this->original_misguess_network->output->acti_vals[0];
				}
			} else {
				this->branch_score_network->activate(state_vals);
				this->original_score_network->activate(state_vals);

				/**
				 * - scale to keep equation running_average_score + scale_factor*score_update = target_val
				 * - also scale average_misguess by abs(scale_factor) to account for potentially lower impact locally
				 *   - these 2 instances of scaling don't directly relate to each other
				 *     - but they cancel out, so don't actually have to do anything
				 */

				double score_diff = this->branch_score_network->output->acti_vals[0]
					- this->original_score_network->output->acti_vals[0];
				double score_val = score_diff / solution->average_misguess;
				if (score_val > 0.1) {
					is_branch = true;
				} else if (score_val < -0.1) {
					is_branch = false;
				} else {
					this->branch_misguess_network->activate(state_vals);
					this->original_misguess_network->activate(state_vals);

					double misguess_diff = this->branch_misguess_network->output->acti_vals[0]
						- this->original_misguess_network->output->acti_vals[0];
					double misguess_val = misguess_diff / solution->misguess_standard_deviation;
					if (misguess_val < -0.1) {
						is_branch = true;
					} else if (misguess_val > 0.1) {
						is_branch = false;
					} else {
						if (rand()%2 == 0) {
							is_branch = true;
						} else {
							is_branch = false;
						}
					}
				}

				if (is_branch) {
					run_helper.running_average_score += run_helper.scale_factor*this->branch_score_update;
					run_helper.running_average_misguess += run_helper.scale_factor*this->branch_misguess_update;
				} else {
					run_helper.running_average_score += run_helper.scale_factor*this->original_score_update;
					run_helper.running_average_misguess += run_helper.scale_factor*this->original_misguess_update;
				}

				if (run_helper.phase == UPDATE_PHASE_NONE
						|| run_helper.phase == EXPLORE_PHASE_EXPERIMENT) {
					BranchNodeHistory* branch_node_history = new BranchNodeHistory(this);
					branch_node_history->is_branch = is_branch;
					node_histories.back().push_back(branch_node_history);
				}

				if (run_helper.phase == UPDATE_PHASE_NONE
						|| run_helper.phase == UPDATE_PHASE_REMEASURE) {
					this->remeasure_counter++;
				}
			}
		}
	} else {
		is_branch = false;

		// don't modify running_average_score/running_average_misguess
	}
}

void BranchNode::backprop(double& scale_factor_error,
						  RunHelper& run_helper,
						  BranchNodeHistory* history) {
	double running_average_score_error = run_helper.target_val - run_helper.running_average_score;
	double running_average_misguess_error = run_helper.final_misguess - run_helper.running_average_misguess;

	double local_score_update;
	double local_misguess_update;
	if (history->is_branch) {
		local_score_update = run_helper.scale_factor*this->branch_score_update;
		local_misguess_update = run_helper.scale_factor*this->branch_misguess_update;
	} else {
		local_score_update = run_helper.scale_factor*this->original_score_update;
		local_misguess_update = run_helper.scale_factor*this->original_misguess_update;
	}

	scale_factor_error += local_score_update*running_average_score_error;
	scale_factor_error += local_misguess_update*running_average_misguess_error;

	run_helper.running_average_score -= local_score_update;
	run_helper.running_average_misguess -= local_misguess_update;

	if (run_helper.phase == UPDATE_PHASE_NONE) {
		double target_score_update = run_helper.target_val - run_helper.running_average_score;
		double target_misguess_update = run_helper.final_misguess - run_helper.running_average_misguess;

		if (history->is_branch) {
			this->branch_score_update = 0.9999*this->branch_score_update + 0.0001*target_score_update;
			this->branch_misguess_update = 0.9999*this->branch_misguess_update + 0.0001*target_misguess_update;
		} else {
			this->original_score_update = 0.9999*this->original_score_update + 0.0001*target_score_update;
			this->original_misguess_update = 0.9999*this->original_misguess_update + 0.0001*target_misguess_update;
		}
	}
}

void BranchNode::remeasure_backprop(RunHelper& run_helper,
									RemeasureBranchNodeHistory* history) {
	double predicted_score = history->running_average_score_snapshot
		+ history->scale_factor_snapshot*history->score_network_output;
	double predicted_score_error = run_helper.target_val - predicted_score;
	ScoreNetwork* score_network = history->score_network_history->network;
	score_network->backprop_weights_with_no_error_signal(
		history->scale_factor_snapshot*predicted_score_error,
		0.002,
		history->score_network_history);

	double predicted_misguess = history->running_average_misguess_snapshot
		+ history->scale_factor_snapshot*history->misguess_network_output;
	double predicted_misguess_error = run_helper.final_misguess - predicted_misguess;
	ScoreNetwork* misguess_network = history->misguess_network_history->network;
	misguess_network->backprop_weights_with_no_error_signal(
		history->scale_factor_snapshot*predicted_misguess_error,
		0.002,
		history->misguess_network_history);

	this->remeasure_counter = 0;
}


