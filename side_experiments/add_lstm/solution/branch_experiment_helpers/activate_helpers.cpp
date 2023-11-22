#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

void BranchExperiment::activate(AbstractNode*& curr_node,
								Problem& problem,
								vector<ContextLayer>& context,
								int& exit_depth,
								AbstractNode*& exit_node,
								RunHelper& run_helper,
								AbstractExperimentHistory*& history) {
	bool is_selected = false;
	if (this->parent_pass_through_experiment != NULL) {
		is_selected = true;
	} else if (run_helper.selected_experiment == NULL) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope->id
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node->id) {
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
				run_helper.experiment_history = new BranchExperimentOverallHistory(this);

				is_selected = true;
			}
		}
	} else if (run_helper.selected_experiment == this) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope->id
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node->id) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			is_selected = true;
		}
	}

	if (is_selected) {
		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(context,
									run_helper);
			break;
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW_PRE:
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node,
							   problem,
							   context,
							   exit_depth,
							   exit_node,
							   run_helper,
							   history);
			break;
		case BRANCH_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 context,
							 exit_depth,
							 exit_node,
							 run_helper);
			break;
		case BRANCH_EXPERIMENT_STATE_VERIFY:
			verify_activate(curr_node,
							problem,
							context,
							exit_depth,
							exit_node,
							run_helper);
			break;
		case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									context,
									exit_depth,
									exit_node,
									run_helper);
			break;
		}
	}
}

void BranchExperiment::hook_helper(vector<int>& scope_context,
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
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void BranchExperiment::hook(vector<ContextLayer>& context) {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_state_nodes[s_index].size(); n_index++) {
			this->new_state_nodes[s_index][n_index]->experiment_state_scope_contexts.push_back(this->new_state_scope_contexts[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_node_contexts.push_back(this->new_state_node_contexts[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_obs_indexes.push_back(this->new_state_obs_indexes[s_index][n_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_defs.push_back(this->new_states[s_index]);
			this->new_state_nodes[s_index][n_index]->experiment_state_network_indexes.push_back(n_index);
		}
	}

	vector<int> scope_context;
	vector<int> node_context;
	hook_helper(scope_context,
				node_context,
				context[context.size() - this->scope_context.size()].temp_state_vals,
				context[context.size() - this->scope_context.size()].scope_history);
}

void BranchExperiment::unhook() {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_state_nodes[s_index].size(); n_index++) {
			this->new_state_nodes[s_index][n_index]->experiment_state_scope_contexts.clear();
			this->new_state_nodes[s_index][n_index]->experiment_state_node_contexts.clear();
			this->new_state_nodes[s_index][n_index]->experiment_state_obs_indexes.clear();
			this->new_state_nodes[s_index][n_index]->experiment_state_defs.clear();
			this->new_state_nodes[s_index][n_index]->experiment_state_network_indexes.clear();
		}
	}
}

void BranchExperiment::backprop(double target_val,
								RunHelper& run_helper,
								BranchExperimentOverallHistory* history) {
	unhook();

	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper,
								history);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 history);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW_PRE:
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val);
		break;
	case BRANCH_EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case BRANCH_EXPERIMENT_STATE_VERIFY:
		verify_backprop(target_val);
		break;
	case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	}

	delete history;
}
