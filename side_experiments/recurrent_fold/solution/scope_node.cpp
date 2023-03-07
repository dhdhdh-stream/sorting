#include "scope_node.h"

using namespace std;

ScopeNode::ScopeNode(vector<bool> pre_state_network_target_is_local,
					 vector<int> pre_state_network_target_indexes,
					 vector<StateNetwork*> pre_state_networks,
					 int inner_scope_id,
					 vector<bool> inner_input_is_local,
					 vector<int> inner_input_indexes,
					 vector<bool> post_state_network_target_is_local,
					 vector<int> post_state_network_target_indexes,
					 vector<StateNetwork*> post_state_networks,
					 StateNetwork* score_network) {
	this->type = NODE_TYPE_INNER_SCOPE;

	this->pre_state_network_target_is_local = pre_state_network_target_is_local;
	this->pre_state_network_target_indexes = pre_state_network_target_indexes;
	this->pre_state_networks = pre_state_networks;

	this->inner_scope_id = inner_scope_id;
	this->inner_input_is_local = inner_input_is_local;
	this->inner_input_indexes = inner_input_indexes;

	this->post_state_network_target_is_local = post_state_network_target_is_local;
	this->post_state_network_target_indexes = post_state_network_target_indexes;
	this->post_state_networks = post_state_networks;

	this->score_network = score_network;
}

ScopeNode::~ScopeNode() {
	for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
		delete this->pre_state_networks[s_index];
	}

	for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
		delete this->post_state_networks[s_index];
	}

	delete this->score_network;
}

void ScopeNode::explore_on_path_activate(vector<double>& local_state_vals,
										 vector<double>& input_vals,
										 vector<vector<double>>& flat_vals,
										 double& predicted_score,
										 double& scale_factor,
										 vector<int>& scope_context,
										 vector<int>& node_context,
										 vector<int>& context_iter,
										 vector<ContextHistory*>& context_histories,
										 int& early_exit_depth,
										 int& early_exit_node_id,
										 FoldHistory*& early_exit_fold_history,
										 int& explore_exit_depth,
										 int& explore_exit_node_id,
										 FoldHistory*& explore_exit_fold_history,
										 RunHelper& run_helper,
										 ScopeNodeHistory* history) {
	// don't save history, as won't backprop
	for (int s_index = 0; s_index < (int)this->pre_state_networks.size(); s_index++) {
		this->pre_state_networks[s_index]->activate(local_state_vals,
													input_vals);
		local_state_vals[this->pre_state_network_target_indexes[s_index]] += this->pre_state_networks[s_index]->output->acti_vals[0];
	}

	Scope* inner_scope = solution->scopes[this->inner_scope_id];
	vector<double> scope_input_vals;
	for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
		if (this->inner_input_is_local[i_index]) {
			scope_input_vals.push_back(local_state_vals[this->inner_input_indexes[i_index]]);
		} else {
			scope_input_vals.push_back(input_vals[this->inner_input_indexes[i_index]]);
		}
	}
	// if inner_scope expanded so there's no matching index, pass 0.0
	for (int i_index = (int)this->inner_input_is_local.size(); i_index < inner_scope->num_input_states; i_index++) {
		scope_input_vals.push_back(0.0);
	}
	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	inner_scope->explore_on_path_activate(scope_input_vals,
										  flat_vals,
										  predicted_score,
										  scale_factor,
										  scope_context,
										  node_context,
										  context_iter,
										  context_histories,
										  early_exit_depth,
										  early_exit_node_id,
										  early_exit_fold_history,
										  explore_exit_depth,
										  explore_exit_node_id,
										  explore_exit_fold_history,
										  run_helper,
										  inner_scope_history);
	history->inner_scope_history = inner_scope_history;
	// even if early exit, set state_vals
	for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
		if (this->inner_input_is_local[i_index]) {
			local_state_vals[this->inner_input_indexes[i_index]] += scope_input_vals[i_index];
		} else {
			input_vals[this->inner_input_indexes[i_index]] += scope_input_vals[i_index];
		}
	}

	if (early_exit_depth == -1 && explore_exit_depth == -1) {
		history->is_early_exit = false;

		for (int s_index = 0; s_index < (int)this->post_state_networks.size(); s_index++) {
			StateNetworkHistory* network_history = new StateNetworkHistory(this->post_state_networks[s_index]);
			this->post_state_networks[s_index]->activate(local_state_vals,
														 input_vals,
														 network_history);
			history->post_state_network_histories.push_back(network_history);
			if (this->post_state_network_target_is_local[s_index]) {
				local_state_vals[this->post_state_network_target_indexes[s_index]] += this->post_state_networks[s_index]->output->acti_vals[0];
			} else {
				input_vals[this->post_state_network_target_indexes[s_index]] += this->post_state_networks[s_index]->output->acti_vals[0];
			}
		}

		StateNetworkHistory* score_network_history = new StateNetworkHistory(this->score_network);
		this->score_network->scope_activate(local_state_vals,
											input_vals,
											score_network_history);
		history->score_network_history = score_network_history;
		history->score_network_update = this->score_network->output->acti_vals[0];
		predicted_score += scale_factor*this->score_network->output->acti_vals[0];
	} else {
		history->is_early_exit = true;
	}
}

void ScopeNode::explore_on_path_backprop(vector<double>& local_state_errors,
										 vector<double>& input_errors,
										 double target_val,
										 double final_misguess,
										 double& predicted_score,
										 double& scale_factor,
										 RunHelper& run_helper,
										 ScopeNodeHistory* history) {
	if (!history->is_early_exit) {
		this->score_network->backprop_errors_with_no_weight_change(
		target_val - predicted_score,
		local_state_errors,
		input_errors,
		history->score_network_history);

		predicted_score -= scale_factor*history->score_network_update;

		for (int s_index = (int)this->post_state_networks.size()-1; s_index >= 0; s_index--) {
			if (this->post_state_network_target_is_local[s_index]) {
				this->post_state_networks[s_index]->backprop_errors_with_no_weight_change(
					local_state_errors[this->post_state_network_target_indexes[s_index]],
					local_state_errors,
					input_errors,
					history->post_state_network_histories[s_index]);
			} else {
				this->post_state_networks[s_index]->backprop_errors_with_no_weight_change(
					input_errors[this->post_state_network_target_indexes[s_index]],
					local_state_errors,
					input_errors,
					history->post_state_network_histories[s_index]);
			}
		}
	}

	Scope* inner_scope = solution->scopes[this->inner_scope_id];
	vector<double> scope_input_errors;
	for (int i_index = 0; i_index < (int)this->inner_input_is_local.size(); i_index++) {
		if (this->inner_input_is_local[i_index]) {
			scope_input_errors.push_back(local_state_errors[this->inner_input_indexes[i_index]]);
		} else {
			scope_input_errors.push_back(input_errors[this->inner_input_indexes[i_index]]);
		}
	}
	for (int i_index = (int)this->inner_input_is_local.size(); i_index < inner_scope->num_input_states; i_index++) {
		scope_input_errors.push_back(0.0);
	}
	inner_scope->explore_on_path_backprop(scope_input_errors,
										  target_val,
										  final_misguess,
										  predicted_score,
										  scale_factor,
										  run_helper,
										  history->inner_scope_history);

	// no further backprop after
}

ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;

	this->score_network_history = NULL;	// may not trigger
}

ScopeNodeHistory::~ScopeNodeHistory() {
	for (int s_index = 0; s_index < (int)this->pre_state_network_histories.size(); s_index++) {
		delete this->pre_state_network_histories[s_index];
	}

	delete this->inner_scope_history;

	for (int s_index = 0; s_index < (int)this->post_state_network_histories.size(); s_index++) {
		delete this->post_state_network_histories[s_index];
	}

	if (this->score_network_history != NULL) {
		delete this->score_network_history;
	}
}
