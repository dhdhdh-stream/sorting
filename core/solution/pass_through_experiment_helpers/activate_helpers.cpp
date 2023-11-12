#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void PassThroughExperiment::activate(AbstractNode*& curr_node,
									 Problem& problem,
									 vector<ContextLayer>& context,
									 int& exit_depth,
									 AbstractNode*& exit_node,
									 RunHelper& run_helper,
									 AbstractExperimentHistory*& history) {
	if (run_helper.selected_experiment == NULL) {
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
			bool select = false;
			set<AbstractExperiment*>::iterator it = run_helper.experiments_seen.find(this);
			if (it == run_helper.experiments_seen.end()) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					select = true;
				}

				run_helper.experiments_seen_order.push_back(this);
				run_helper.experiments_seen.insert(this);
			}
			if (select) {
				hook(context);

				run_helper.selected_experiment = this;
				PassThroughExperimentOverallHistory* overall_history = new PassThroughExperimentOverallHistory(this);
				run_helper.experiment_history = overall_history;

				switch (this->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
					measure_existing_score_activate(context);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
					if (this->sub_state_iter == 0) {
						this->sub_state_iter = -1;

						explore_initial_activate(curr_node,
												 problem,
												 context,
												 exit_depth,
												 exit_node,
												 run_helper);

						this->sub_state_iter = 0;
					} else {
						explore_activate(curr_node,
										 problem,
										 context,
										 exit_depth,
										 exit_node,
										 run_helper);
					}
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
					measure_new_score_activate(curr_node,
											   problem,
											   context,
											   exit_depth,
											   exit_node,
											   run_helper,
											   history);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS:
					measure_existing_misguess_activate(context);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW_MISGUESS:
					train_new_misguess_activate(curr_node,
												problem,
												context,
												exit_depth,
												exit_node,
												run_helper,
												history);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_MISGUESS:
					measure_new_misguess_activate(curr_node,
												  problem,
												  context,
												  exit_depth,
												  exit_node,
												  run_helper,
												  history);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
					overall_history->branch_experiment_history = new BranchExperimentOverallHistory(this->branch_experiment);

					experiment_activate(curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper,
										history);

					break;
				}
			}
		}
	} else if (run_helper.selected_experiment == this) {
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
			switch (this->state) {
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
				measure_existing_score_activate(context);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
				if (this->sub_state_iter != -1) {
					/**
					 * - handle edge case where hit in create_sequence()
					 */
					explore_activate(curr_node,
									 problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper);
				}

				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
				measure_new_score_activate(curr_node,
										   problem,
										   context,
										   exit_depth,
										   exit_node,
										   run_helper,
										   history);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS:
				measure_existing_misguess_activate(context);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW_MISGUESS:
				train_new_misguess_activate(curr_node,
											problem,
											context,
											exit_depth,
											exit_node,
											run_helper,
											history);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_MISGUESS:
				measure_new_misguess_activate(curr_node,
											  problem,
											  context,
											  exit_depth,
											  exit_node,
											  run_helper,
											  history);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
				experiment_activate(curr_node,
									problem,
									context,
									exit_depth,
									exit_node,
									run_helper,
									history);
				break;
			}
		}
	}
}

void PassThroughExperiment::hook_helper(vector<int>& scope_context,
										vector<int>& node_context,
										map<State*, StateStatus>& temp_state_vals,
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
		this->branch_experiment->hook(context);
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

				action_node->experiment_state_scope_contexts.clear();
				action_node->experiment_state_node_contexts.clear();
				action_node->experiment_state_defs.clear();
				action_node->experiment_state_network_indexes.clear();
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->new_state_nodes[s_index][n_index];

				scope_node->experiment_state_scope_contexts.clear();
				scope_node->experiment_state_node_contexts.clear();
				scope_node->experiment_state_obs_indexes.clear();
				scope_node->experiment_state_defs.clear();
				scope_node->experiment_state_network_indexes.clear();
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
		measure_existing_misguess_parent_scope_end_activate(
			context,
			run_helper);
		break;
	}
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper,
									 PassThroughExperimentOverallHistory* history) {
	unhook();

	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_backprop(target_val,
										run_helper,
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
		measure_existing_misguess_backprop(target_val,
										   run_helper,
										   history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW_MISGUESS:
		train_new_misguess_backprop(target_val,
									history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_MISGUESS:
		measure_new_misguess_backprop(target_val,
									  history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
		experiment_backprop(target_val,
							run_helper,
							history);
		break;
	}

	delete history;
}
