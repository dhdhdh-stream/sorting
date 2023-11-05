#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::activate(int& curr_node_id,
									 Problem& problem,
									 vector<ContextLayer>& context,
									 int& exit_depth,
									 int& exit_node_id,
									 RunHelper& run_helper,
									 AbstractExperimentHistory*& history) {
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
		if (run_helper.selected_experiment == this) {
			switch (this->state) {
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
				measure_existing_score_activate(context);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
				explore_activate(curr_node_id,
								 problem,
								 context,
								 exit_depth,
								 exit_node_id,
								 run_helper);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
				measure_new_score_activate(curr_node_id,
										   problem,
										   context,
										   exit_depth,
										   exit_node_id,
										   run_helper,
										   history);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS:

				break;
			case PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW_MISGUESS:
				train_activate(curr_node_id,
							   problem,
							   context,
							   exit_depth,
							   exit_node_id,
							   run_helper,
							   history);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_MISGUESS:

				break;
			case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:

				break;
			}
		} else if (run_helper.selected_experiment == NULL) {
			bool select = false;
			map<AbstractExperiment*, int>::iterator it = run_helper.experiments_seen.find(this);
			if (it == run_helper.experiments_seen_counts.end()) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					select = true;
				}
			}
			if (select) {
				hook(context);

				run_helper.select_experiment = this;
				run_helper.experiment_history = new PassThroughExperimentOverallHistory(this);

				switch (this->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
					measure_existing_score_activate(context);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
					if (this->sub_state_iter == 0) {
						explore_initial_activate(curr_node_id,
												 problem,
												 context,
												 exit_depth,
												 exit_node_id,
												 run_helper);
					} else {
						explore_activate(curr_node_id,
										 problem,
										 context,
										 exit_depth,
										 exit_node_id,
										 run_helper);
					}
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
					measure_new_score_activate(curr_node_id,
											   problem,
											   context,
											   exit_depth,
											   exit_node_id,
											   run_helper,
											   history);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS:

					break;
				case PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW_MISGUESS:

					break;
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_MISGUESS:

					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:

					break;
				}
			} else {
				if (it == run_helper.experiments_seen_counts.end()) {
					run_helper.experiments_seen_order.push_back(this);
					run_helper.experiments_seen_counts[this] = 1;
				} else {
					it->second++;
				}
			}
		}
	}
}

void PassThroughExperiment::hook_helper(vector<int>& scope_context,
										vector<int>& node_context,
										map<int, StateStatus>& temp_state_vals,
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
													  temp_state_vals,
													  action_node_history);
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				hook_helper(scope_context,
							node_context,
							temp_state_vals,
							scope_node_history->inner_scope_history);

				node_context.back() = -1;

				scope_node->experiment_back_activate(scope_context,
													 node_context,
													 temp_state_vals,
													 scope_node_history);
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void PassThroughExperiment::hook(vector<ContextLayer>& context) {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_state_nodes[s_index].size(); n_index++) {
			if (this->new_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->new_state_nodes[s_index][n_index];

				action_node->experiment_state_scope_contexts.push_back(this->new_state_scope_contexts[s_index][n_index]);
				action_node->experiment_state_node_contexts.push_back(this->new_state_node_contexts[s_index][n_index]);
				action_node->experiment_state_defs.push_back(this->new_states[s_index]);
				action_node->experiment_state_network_indexes.push_back(n_index);
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->new_state_nodes[s_index][n_index];

				scope_node->experiment_state_scope_contexts.push_back(this->new_state_scope_contexts[s_index][n_index]);
				scope_node->experiment_state_node_contexts.push_back(this->new_state_node_contexts[s_index][n_index]);
				scope_node->experiment_state_obs_indexes.push_back(this->new_state_obs_indexes[s_index][n_index]);
				scope_node->experiment_state_defs.push_back(this->new_states[s_index]);
				scope_node->experiment_state_network_indexes.push_back(n_index);
			}
		}
	}

	if (this->state == PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT) {
		this->branch_experiment->hook();
	}

	vector<int> scope_context;
	vector<int> node_context;
	hook_helper(scope_context,
				node_context,
				context[context.size() - this->scope_context.size()].temp_state_vals,
				context[context.size() - this->scope_context.size()].scope_history);
}

void PassThroughExperiment::unhook() {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_state_nodes[s_index].size(); n_index++) {
			if (this->new_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->new_state_nodes[s_index][n_index];

				action_node->experiment_hook_state_scope_contexts.clear();
				action_node->experiment_hook_state_node_contexts.clear();
				action_node->experiment_hook_state_defs.clear();
				action_node->experiment_hook_state_network_indexes.clear();
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->new_state_nodes[s_index][n_index];

				scope_node->experiment_hook_state_scope_contexts.clear();
				scope_node->experiment_hook_state_node_contexts.clear();
				scope_node->experiment_hook_state_obs_indexes.clear();
				scope_node->experiment_hook_state_defs.clear();
				scope_node->experiment_hook_state_network_indexes.clear();
			}
		}
	}

	if (this->state == PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT) {
		this->branch_experiment->unhook();
	}
}

void PassThroughExperiment::parent_scope_end_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* parent_scope_history) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_parent_scope_end_activate(
			context,
			run_helper,
			parent_scope_history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS:

		break;
	}
}

void PassThroughExperiment::backprop(double target_val,
									 PassThroughExperimentOverallHistory* history) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_backprop(target_val,
										history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
		measure_new_score_backprop(target_val,
								   history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS:

		break;
	case PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW_MISGUESS:

		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_MISGUESS:

		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:

		break;
	}
}
