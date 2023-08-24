#include "branch_node.h"

using namespace std;



void BranchNode::activate_backfill_helper(
		bool on_path,
		int state_index,
		int& next_state_index,
		double& state_val,
		ScopeHistory* scope_history) {
	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				if (action_node->state_networks[state_index] != NULL) {
					StateNetwork* network = action_node->state_networks[state_index];
					if (action_node_history->state_network_histories.size() != 0) {
						StateNetworkHistory* network_history = new StateNetworkHistory(network);
						network->activate(action_node_history->obs_snapshot,
										  state_val,
										  network_history);
						action_node_history->state_network_histories[state_index] = network_history;
					} else {
						network->activate(action_node_history->obs_snapshot,
										  state_val);
					}
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				int input_index = scope_node->state_to_input_mapping[state_index];
				if (input_index != -1) {
					if (on_path
							&& i_index == (int)scope_history->node_histories.size()-1
							&& h_index == (int)scope_history->node_histories[i_index].size()-1) {
						next_state_index = input_index;
					} else {
						activate_backfill_helper(false,
												 input_index,
												 next_state_index,
												 state_val,
												 scope_node_history->inner_scope_history);
					}
				}
			}
		}
	}
}

void BranchNode::activate(vector<ForwardContextLayer>& context,
						  RunHelper& run_helper,
						  BranchNodeHistory* history) {
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
		for (int b_index = 0; b_index < (int)this->backfill_start_layer.size(); b_index++) {
			int starting_context_index;
			if (this->backfill_start_layer[b_index] == 0) {
				starting_context_index = 0;
			} else {
				starting_context_index = context.size()-this->scope_context.size()+this->backfill_start_layer[b_index]-1;
			}
			if (!context[starting_context_index].states_initialized[this->backfill_state_index[b_index]]) {
				double state_val = 0.0;
				int curr_state_index = this->backfill_state_index[b_index];
				// include last (i.e., current) layer
				for (int c_index = starting_context_index; c_index < (int)context.size(); c_index++) {
					context[c_index].states_initialized[curr_state_index] = true;

					int next_state_index,
					activate_backfill_helper(true,
											 curr_state_index,
											 next_state_index,
											 state_val,
											 context[c_index].scope_history);

					// no need to set context[c_index].state_vals

					curr_state_index = next_state_index;
				}
				context.back().state_vals->at(curr_state_index) = state_val;
				context.back().states_initialized[curr_state_index] = true;
			}
		}

		if (this->branch_is_pass_through) {
			history->is_branch = true;
		} else {
			if (run_helper.phase == RUN_PHASE_UPDATE_NONE
					&& this->remeasure_counter > solution->max_decisions) {


			}


			if (run_helper.phase == RUN_PHASE_UPDATE_NONE
					|| run_helper.phase == RUN_PHASE_UPDATE_REMEASURE) {
				this->remeasure_counter++;
			}
		}
	} else {
		history->is_branch = false;
	}
}



void BranchNode::remeasure_backprop(RunHelper& run_helper,
									BranchNodeHistory* history) {
	double branch_predicted_score = history->predicted_score_snapshot
		+ history->scale_factor_snapshot*history->score_network_output;
	// HERE
	ScoreNetwork* score_network = history->score_network_history->network;
	score_network->backprop_weights_with_no_error_signal(
		)
}
