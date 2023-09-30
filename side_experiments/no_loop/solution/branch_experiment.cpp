#include "branch_experiment.h"

using namespace std;

const int MEASURE_ITERS = 10000;

const double MIN_SCORE_IMPACT = 0.05;

void BranchExperiment::hook(vector<ContextLayer>& context,
							RunHelper& run_helper,
							BranchExperimentHistory* history) {
	run_helper.branch_experiment_history = history;

	context[context.size() - this->scope_context.size()]
		.scope_history->inner_branch_experiment_history = history;

	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_score_state_nodes[s_index].size(); n_index++) {
			if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->new_score_state_nodes[s_index][n_index];

				action_node->experiment_hook_score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				action_node->experiment_hook_score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				action_node->experiment_hook_score_state_defs.push_back(this->new_score_states[s_index]);
				action_node->experiment_hook_score_state_network_indexes.push_back(n_index);
			} else if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)this->new_score_state_nodes[s_index][n_index];

				scope_node->experiment_hook_score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				scope_node->experiment_hook_score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				scope_node->experiment_hook_score_state_obs_indexes.push_back(this->new_score_state_obs_indexes[s_index][n_index]);
				scope_node->experiment_hook_score_state_defs.push_back(this->new_score_states[s_index]);
				scope_node->experiment_hook_score_state_network_indexes.push_back(n_index);
			} else {
				BranchNode* branch_node = (BranchNode*)this->new_score_state_nodes[s_index][n_index];

				branch_node->experiment_hook_score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				branch_node->experiment_hook_score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				branch_node->experiment_hook_score_state_defs.push_back(this->new_score_states[s_index]);
				branch_node->experiment_hook_score_state_network_indexes.push_back(n_index);
			}
		}
	}

	if (this->obs_experiment != NULL) {
		ObsExperimentHistory* obs_experiment_history = new ObsExperimentHistory(this->obs_experiment);
		this->obs_experiment->hook(obs_experiment_history);
		history->obs_experiment_history = obs_experiment_history;
	}
}

void BranchExperiment::activate_helper(vector<int>& scope_context,
									   vector<int>& node_context,
									   BranchExperimentHistory* history,
									   ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->experiment_back_activate(scope_context,
													  node_context,
													  history,
													  action_node_history);
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				activate_helper(scope_context,
								node_context,
								history,
								scope_node_history->inner_scope_history);

				node_context.back() = -1;

				scope_node->experiment_back_activate(scope_context,
													 node_context,
													 history,
													 scope_node_history);
			} else {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[i_index][h_index];
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				branch_node->experiment_back_activate(scope_context,
													  node_context,
													  history,
													  branch_node_history);
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void BranchExperiment::train_activate(int& curr_node_id,
									  vector<double>& flat_vals,
									  vector<ContextLayer>& context,
									  int& exit_depth,
									  int& exit_node_id,
									  RunHelper& run_helper,
									  BranchExperimentHistory* history) {
	hook(context,
		 run_helper,
		 history);

	vector<int> scope_context;
	vector<int> node_context;
	activate_helper(scope_context,
					node_context,
					history,
					context[context.size()-this->scope_context.size()].scope_history);

	history->sequence_histories = vector<SequenceHistory*>(this->step_types.size(), NULL);
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			this->actions[s_index]->branch_experiment_activate(
				flat_vals,
				context,
				history);
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->sequences[s_index]);
			history->sequence_histories[s_index] = sequence_history;
			this->sequences[s_index]->activate(flat_vals,
											   context,
											   run_helper,
											   sequence_history);
		}
	}

	if (this->exit_depth == 0) {
		curr_node_id = this->exit_node_id;
	} else {
		exit_depth = this->exit_depth;
		exit_node_id = this->exit_node_id;
	}
}

void BranchExperiment::unhook(BranchExperimentHistory* history) {
	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_score_state_nodes[s_index].size(); n_index++) {
			if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->new_score_state_nodes[s_index][n_index];

				action_node->experiment_hook_score_state_scope_contexts.clear();
				action_node->experiment_hook_score_state_node_contexts.clear();
				action_node->experiment_hook_score_state_defs.clear();
				action_node->experiment_hook_score_state_network_indexes.clear();
			} else if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)this->new_score_state_nodes[s_index][n_index];

				scope_node->experiment_hook_score_state_scope_contexts.clear();
				scope_node->experiment_hook_score_state_node_contexts.clear();
				scope_node->experiment_hook_score_state_obs_indexes.clear();
				scope_node->experiment_hook_score_state_defs.clear();
				scope_node->experiment_hook_score_state_network_indexes.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->new_score_state_nodes[s_index][n_index];

				branch_node->experiment_hook_score_state_scope_contexts.clear();
				branch_node->experiment_hook_score_state_node_contexts.clear();
				branch_node->experiment_hook_score_state_defs.clear();
				branch_node->experiment_hook_score_state_network_indexes.clear();
			}
		}
	}

	if (this->obs_experiment != NULL) {
		this->obs_experiment->unhook(history->obs_experiment_history);
	}
}

void BranchExperiment::train_backprop(double target_val,
									  BranchExperimentHistory* history) {
	unhook(history);

	double predicted_score = this->average_score;

	for (map<State*, StateStatus>::iterator it = history->parent_score_state_snapshots.begin();
			it != history->parent_score_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			// set for backprop in the following
			it->second.val *= it->first->resolved_standard_deviation;
		} else if (it->first->resolved_networks.find(last_network) == it->first->resolved_networks.end()) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
		}
		map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
		if (scale_it == this->score_state_scales.end()) {
			scale_it = this->score_state_scales.insert({it->first, Scale(it->first->scale->weight)}).first;
		}
		predicted_score += scale_it->second->weight * it->second.val;
	}

	for (map<State*, StateStatus>::iterator it = history->new_score_state_vals.begin();
			it != history->new_score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_networks.find(last_network) == it->first->resolved_networks.end()) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
		}
		predicted_score += it->first->scale->weight * it->second.val;
	}

	double error = target_val - predicted_score;

	for (map<State*, StateStatus>::iterator it = history->parent_score_state_snapshots.begin();
			it != history->parent_score_state_snapshots.end(); it++) {
		map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
		scale_it->second->backprop(it->second.val*error, 0.001);
	}

	for (map<State*, StateStatus>::iterator it = history->new_score_state_vals.begin();
			it != history->new_score_state_vals.end(); it++) {
		it->first->scale->backprop(it->second.val*error, 0.001);

		// don't worry about removing score states for now
	}

	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
	this->average_misguess = 0.9999*this->average_misguess + 0.0001*curr_misguess;
	double curr_misguess_variance = (this->average_misguess - curr_misguess) * (this->average_misguess - curr_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	if (history->obs_experiment_history != NULL) {
		this->obs_experiment->backprop(history->obs_experiment_history,
									   target_val,
									   predicted_score);
	}
}

void BranchExperiment::measure_activate(int& curr_node_id,
										vector<double>& flat_vals,
										vector<ContextLayer>& context,
										int& exit_depth,
										int& exit_node_id,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	hook(context,
		 run_helper,
		 history);

	vector<int> scope_context;
	vector<int> node_context;
	activate_helper(scope_context,
					node_context,
					history,
					context[context.size()-this->scope_context.size()].scope_history);

	double branch_score = 0.0;
	double original_score = 0.0;

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network == NULL) {
			map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
			if (scale_it != this->score_state_scales.end()) {
				branch_score += it->second.val
					* it->first->resolved_standard_deviation
					* scale_it->second->weight;
			}
			original_score += it->second.val
				* it->first->resolved_standard_deviation
				* it->first->scale->weight;
		} else if (it->first->resolved_networks.find(last_network) == it->first->resolved_networks.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
			if (scale_it != this->score_state_scales.end()) {
				branch_score += normalized * scale_it->second->weight;
			}
			original_score += normalized * it->first->scale->weight;
		} else {
			map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
			if (scale_it != this->score_state_scales.end()) {
				branch_score += it->second.val * scale_it->second->weight;
			}
			original_score += it->second.val * it->first->scale->weight;
		}
	}

	for (map<State*, StateStatus>::iterator it = history->new_score_state_vals.begin();
			it != history->new_score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_networks.find(last_network) == it->first->resolved_networks.end()) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
			branch_score += normalized * it->first->scale->weight;
		} else {
			branch_score += it->second.val * it->first->scale_weight;
		}
	}

	unhook(history);
	// don't need to track score state anymore for evaluate

	if (branch_score > original_score) {
		this->branch_count++;

		for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
			if (this->step_types[s_index] == STEP_TYPE_ACTION) {
				this->actions[s_index]->branch_experiment_activate(
					flat_vals,
					context,
					history);
			} else {
				SequenceHistory* sequence_history = new SequenceHistory(this->sequences[s_index]);
				this->sequences[s_index]->activate(flat_vals,
												   context,
												   run_helper,
												   sequence_history);
				delete sequence_history;
			}
		}

		if (this->exit_depth == 0) {
			curr_node_id = this->exit_node_id;
		} else {
			exit_depth = this->exit_depth;
			exit_node_id = this->exit_node_id;
		}
	}
}

void BranchExperiment::measure_backprop(double target_val,
										BranchExperimentHistory* history) {
	this->combined_score += target_val;
}

void BranchExperiment::eval() {
	Scope* parent = solution->scopes[this->scope_context[0]];

	double combined_average_score = this->combined_score / MEASURE_ITERS;
	double combined_improvement = combined_average_score - parent->average_score;
	double score_standard_deviation = sqrt(parent->score_variance);
	double combined_improvement_t_score = combined_improvement
		/ score_standard_deviation / sqrt(MEASURE_ITERS);
	if (combined_improvement_t_score > 2.326) {		// >99%
		double branch_weight = this->branch_count / MEASURE_ITERS;
		if (branch_weight > 0.98) {

		} else {
			Scope* starting_scope = solution->scopes[this->scope_context.back()];
			Scope* parent_scope = solution->scopes[this->scope_context[0]];

			BranchNode* new_branch_node = new BranchNode();
			new_branch_node->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(new_branch_node);

			new_branch_node->branch_scope_context = this->scope_context;
			new_branch_node->branch_node_context = this->node_context;
			new_branch_node->branch_node_context.back() = new_branch_node->id;

			new_branch_node->branch_is_pass_through = false;

			// TODO: add nodes first, so when states

			for (map<State*, Scale*>::iterator it = this->score_state_scales.begin();
					it != this->score_state_scales.end(); it++) {
				double original_impact = it->first->resolved_standard_deviation * it->first->scale->weight;
				double new_impact = it->first->resolved_standard_deviation * it->second->weight;
				if (abs(original_impact - new_impact) > MIN_SCORE_IMPACT*score_standard_deviation) {

				}
			}


		}
	} else {
		// 0.0001 rolling average variance approx. equal to 20000 average variance (?)

		double score_improvement = this->average_score - parent->average_score;
		double score_improvement_t_score = score_improvement
			/ score_standard_deviation / sqrt(20000);

		double misguess_improvement = parent->average_misguess - this->average_misguess;
		double misguess_standard_deviation = sqrt(parent->misguess_variance);
		double misguess_improvement_t_score = misguess_improvement
			/ misguess_standard_deviation / sqrt(20000);

		if (score_improvement_t_score > -0.674	// 75%<
				&& misguess_improvement_t_score > 2.326) {

		} else {
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->actions[s_index];
				} else {
					delete this->sequences[s_index];
				}
			}

			for (map<State*, Scale*>::iterator it = this->score_state_scales.begin();
					it != this->score_state_scales.end(); it++) {
				delete it->second;
			}

			for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
				delete this->new_score_states[s_index];
			}
		}
	}
}
