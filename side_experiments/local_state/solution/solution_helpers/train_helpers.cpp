#include "solution_helpers.h"

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "pass_through_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "transition_network.h"

using namespace std;

void backprop_helper(TrainScopeHistory* scope_history,
					 Eigen::VectorXf& state_error,
					 double target_val) {
	Scope* scope = scope_history->scope;

	scope->end_score_network->load(scope_history->end_score_network_history);
	scope->end_score_network->backprop(target_val,
									   state_error);

	for (int h_index = (int)scope_history->node_histories.size()-1; h_index >= 0; h_index--) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_NOOP:
			{
				TrainNoopNodeHistory* noop_node_history = (TrainNoopNodeHistory*)scope_history->node_histories[h_index];
				NoopNode* noop_node = (NoopNode*)node;
				noop_node->score_network->load(noop_node_history->score_network_history);
				noop_node->score_network->backprop(target_val,
												   state_error);
			}
			break;
		case NODE_TYPE_ACTION:
			{
				TrainActionNodeHistory* action_node_history = (TrainActionNodeHistory*)scope_history->node_histories[h_index];
				ActionNode* action_node = (ActionNode*)node;
				if (!action_node->is_generic) {
					action_node->score_network->load(action_node_history->score_network_history);
					action_node->score_network->backprop(target_val,
														 state_error);
				}
				for (int h_index = (int)action_node_history->init_network_histories.size()-1; h_index >= 0; h_index--) {
					if (action_node_history->init_network_histories[h_index] != NULL) {
						action_node->init_networks[h_index]->load(action_node_history->init_network_histories[h_index]);
						action_node->init_networks[h_index]->backprop(state_error);
					}
				}
				action_node->obs_network->load(action_node_history->obs_network_history);
				action_node->obs_network->backprop(state_error);
				action_node->action_network->load(action_node_history->action_network_history);
				action_node->action_network->backprop(state_error);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				TrainScopeNodeHistory* scope_node_history = (TrainScopeNodeHistory*)scope_history->node_histories[h_index];
				ScopeNode* scope_node = (ScopeNode*)node;

				scope_node->score_network->load(scope_node_history->score_network_history);
				scope_node->score_network->backprop(target_val,
													state_error);

				Eigen::VectorXf inner_state_error;
				inner_state_error.resize(scope_node->scope->num_states);
				inner_state_error.setConstant(0.0);

				if (!scope_node_history->out_is_drop) {
					scope_node->out_network->load(scope_node_history->out_network_history);
					scope_node->out_network->backprop(state_error,
													  inner_state_error);
					for (int n_index = scope_node->out_pass_through_networks.size()-1; n_index >= 0; n_index--) {
						PassThroughNetwork* pass_through_network = scope_node->out_pass_through_networks[n_index];
						double error = state_error(pass_through_network->back_state_index);
						inner_state_error(pass_through_network->front_state_index) += error;
					}
				}

				backprop_helper(scope_node_history->scope_history,
								inner_state_error,
								target_val);

				if (!scope_node_history->in_is_drop) {
					scope_node->in_network->load(scope_node_history->in_network_history);
					scope_node->in_network->backprop(inner_state_error,
													 state_error);
					for (int n_index = scope_node->in_pass_through_networks.size()-1; n_index >= 0; n_index--) {
						PassThroughNetwork* pass_through_network = scope_node->in_pass_through_networks[n_index];
						double error = inner_state_error(pass_through_network->back_state_index);
						state_error(pass_through_network->front_state_index) += error;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				TrainBranchNodeHistory* branch_node_history = (TrainBranchNodeHistory*)scope_history->node_histories[h_index];
				if (branch_node_history->score_network_history != NULL) {
					BranchNode* branch_node = (BranchNode*)node;
					if (branch_node_history->is_branch) {
						branch_node->branch_network->load(branch_node_history->score_network_history);
						branch_node->branch_network->backprop(target_val,
															  state_error);
					} else {
						branch_node->original_network->load(branch_node_history->score_network_history);
						branch_node->original_network->backprop(target_val,
																state_error);
					}
				}
			}
			break;
		}
	}

	if (!scope_history->is_drop) {
		for (int h_index = (int)scope_history->start_init_network_histories.size()-1; h_index >= 0; h_index--) {
			if (scope_history->start_init_network_histories[h_index] != NULL) {
				scope->start_init_networks[h_index]->load(scope_history->start_init_network_histories[h_index]);
				scope->start_init_networks[h_index]->backprop(state_error);
			}
		}

		scope->start_obs_network->load(scope_history->start_obs_network_history);
		scope->start_obs_network->backprop(state_error);
	}
}

void update_helper(TrainScopeHistory* scope_history,
				   int iter_index) {
	Scope* scope = scope_history->scope;

	if (!scope_history->is_drop) {
		for (int n_index = 0; n_index < (int)scope_history->start_init_network_histories.size(); n_index++) {
			if (scope_history->start_init_network_histories[n_index] != NULL) {
				if (scope->start_init_networks[n_index]->last_update_iter != iter_index) {
					scope->start_init_networks[n_index]->update();

					scope->start_init_networks[n_index]->last_update_iter = iter_index;
				}
			}
		}

		if (scope->start_obs_network->last_update_iter != iter_index) {
			scope->start_obs_network->update();

			scope->start_obs_network->last_update_iter = iter_index;
		}
	}

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)node;

				if (noop_node->score_network->last_update_iter != iter_index) {
					noop_node->score_network->update();

					noop_node->score_network->last_update_iter = iter_index;
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				TrainActionNodeHistory* action_node_history = (TrainActionNodeHistory*)scope_history->node_histories[h_index];
				ActionNode* action_node = (ActionNode*)node;

				if (action_node->action_network->last_update_iter != iter_index) {
					action_node->action_network->update();

					action_node->action_network->last_update_iter = iter_index;
				}

				if (action_node->obs_network->last_update_iter != iter_index) {
					action_node->obs_network->update();

					action_node->obs_network->last_update_iter = iter_index;
				}

				for (int n_index = 0; n_index < (int)action_node_history->init_network_histories.size(); n_index++) {
					if (action_node_history->init_network_histories[n_index] != NULL) {
						if (action_node->init_networks[n_index]->last_update_iter != iter_index) {
							action_node->init_networks[n_index]->update();

							action_node->init_networks[n_index]->last_update_iter = iter_index;
						}
					}
				}

				if (!action_node->is_generic) {
					if (action_node->score_network->last_update_iter != iter_index) {
						action_node->score_network->update();

						action_node->score_network->last_update_iter = iter_index;
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				TrainScopeNodeHistory* scope_node_history = (TrainScopeNodeHistory*)scope_history->node_histories[h_index];
				ScopeNode* scope_node = (ScopeNode*)node;

				if (!scope_node_history->in_is_drop) {
					if (scope_node->in_network->last_update_iter != iter_index) {
						scope_node->in_network->update();

						scope_node->in_network->last_update_iter = iter_index;
					}
				}

				update_helper(scope_node_history->scope_history,
							  iter_index);

				if (!scope_node_history->out_is_drop) {
					if (scope_node->out_network->last_update_iter != iter_index) {
						scope_node->out_network->update();

						scope_node->out_network->last_update_iter = iter_index;
					}
				}

				if (scope_node->score_network->last_update_iter != iter_index) {
					scope_node->score_network->update();

					scope_node->score_network->last_update_iter = iter_index;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				TrainBranchNodeHistory* branch_node_history = (TrainBranchNodeHistory*)scope_history->node_histories[h_index];
				if (branch_node_history->score_network_history != NULL) {
					BranchNode* branch_node = (BranchNode*)node;
					if (branch_node_history->is_branch) {
						if (branch_node->branch_network->last_update_iter != iter_index) {
							branch_node->branch_network->update();

							branch_node->branch_network->last_update_iter = iter_index;
						}
					} else {
						if (branch_node->original_network->last_update_iter != iter_index) {
							branch_node->original_network->update();

							branch_node->original_network->last_update_iter = iter_index;
						}
					}
				}
			}
			break;
		}
	}

	if (scope_history->end_score_network_history != NULL) {
		if (scope->end_score_network->last_update_iter != iter_index) {
			scope->end_score_network->update();

			scope->end_score_network->last_update_iter = iter_index;
		}
	}
}

void train_helper(SolutionWrapper* wrapper) {
	uniform_int_distribution<int> sample_distribution(0, wrapper->train_scope_histories.size()-1);
	uniform_int_distribution<int> allow_drop_distribution(0, 1);
	for (int iter_index = 0; iter_index < ITERS_PER_BATCH; iter_index++) {
		int index = sample_distribution(generator);

		bool allow_drop = allow_drop_distribution(generator) == 0;
		Eigen::VectorXf state;
		state.resize(wrapper->solution->starting_scope->num_states);
		state.setConstant(0.0);
		TrainScopeHistory* train_scope_history = new TrainScopeHistory(wrapper->solution->starting_scope);
		wrapper->solution->starting_scope->train_activate(
			wrapper->train_scope_histories[index],
			allow_drop,
			state,
			train_scope_history);

		Eigen::VectorXf state_error;
		state_error.resize(wrapper->solution->starting_scope->num_states);
		state_error.setConstant(0.0);
		backprop_helper(train_scope_history,
						state_error,
						wrapper->train_target_val_histories[index]);

		update_helper(train_scope_history,
					  wrapper->train_iter_index);
		wrapper->train_iter_index++;

		delete train_scope_history;
	}

	for (int h_index = 0; h_index < (int)wrapper->train_scope_histories.size(); h_index++) {
		delete wrapper->train_scope_histories[h_index];
	}
	wrapper->train_scope_histories.clear();
	wrapper->train_target_val_histories.clear();
}
