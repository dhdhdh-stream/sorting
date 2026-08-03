#include "solution_helpers.h"

#include <iostream>

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

void update_helper(ScopeHistory* scope_history) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)h_it->second->node;
				noop_node->curr_num_instances++;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)h_it->second->node;
				if (!action_node->is_generic) {
					action_node->curr_num_instances++;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				update_helper(scope_node_history->scope_history);

				scope_node->curr_num_instances++;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					branch_node->branch_curr_num_instances++;
				} else {
					branch_node->original_curr_num_instances++;
				}
			}
			break;
		}
	}
}

void backprop_helper(ScopeHistory* scope_history,
					 vector<Eigen::VectorXf>& state_errors,
					 double target_val,
					 SolutionWrapper* wrapper) {
	Scope* scope = scope_history->scope;

	if (scope_history->end_score_network_history != NULL) {
		scope->end_score_network->load(scope_history->end_score_network_history);
		scope->end_score_network->backprop(target_val,
										   state_errors.back());
	}

	vector<AbstractNodeHistory*> in_order(scope_history->node_histories.size());
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		in_order[it->second->index] = it->second;
	}

	for (int h_index = (int)in_order.size()-1; h_index >= 0; h_index--) {
		AbstractNode* node = in_order[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)in_order[h_index];
				if (!action_node_history->is_drop) {
					ActionNode* action_node = (ActionNode*)node;
					for (int h_index = (int)action_node_history->init_network_histories.size()-1; h_index >= 0; h_index--) {
						if (action_node_history->init_network_histories[h_index] != NULL) {
							action_node->init_networks[h_index]->load(action_node_history->init_network_histories[h_index]);
							action_node->init_networks[h_index]->backprop(state_errors.back());
						}
					}
					action_node->obs_network->load(action_node_history->obs_network_history);
					action_node->obs_network->backprop(state_errors.back());
					action_node->action_network->load(action_node_history->action_network_history);
					action_node->action_network->backprop(state_errors.back());
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)in_order[h_index];
				ScopeNode* scope_node = (ScopeNode*)node;

				state_errors.push_back(Eigen::VectorXf());
				state_errors.back().resize(scope_node->scope->num_states);
				state_errors.back().setConstant(0.0);

				if (!scope_node_history->out_is_drop) {
					scope_node->out_network->load(scope_node_history->out_network_history);
					scope_node->out_network->backprop(state_errors[state_errors.size()-2],
													  state_errors.back());
					for (int n_index = scope_node->out_pass_through_networks.size()-1; n_index >= 0; n_index--) {
						PassThroughNetwork* pass_through_network = scope_node->out_pass_through_networks[n_index];
						double error = state_errors[state_errors.size()-2][pass_through_network->back_state_index];
						state_errors.back()[pass_through_network->front_state_index] += error;
					}
				}

				backprop_helper(scope_node_history->scope_history,
								state_errors,
								target_val,
								wrapper);

				if (!scope_node_history->in_is_drop) {
					scope_node->in_network->load(scope_node_history->in_network_history);
					scope_node->in_network->backprop(state_errors.back(),
													 state_errors[state_errors.size()-2]);
					for (int n_index = scope_node->in_pass_through_networks.size()-1; n_index >= 0; n_index--) {
						PassThroughNetwork* pass_through_network = scope_node->in_pass_through_networks[n_index];
						double error = state_errors.back()[pass_through_network->back_state_index];
						state_errors[state_errors.size()-2][pass_through_network->front_state_index] += error;
					}
				}

				state_errors.pop_back();
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)in_order[h_index];
				BranchNode* branch_node = (BranchNode*)node;
				if (branch_node_history->is_branch) {
					branch_node->branch_network->load(branch_node_history->score_network_history);
					branch_node->branch_network->backprop(target_val,
														  state_errors.back());
				} else {
					branch_node->original_network->load(branch_node_history->score_network_history);
					branch_node->original_network->backprop(target_val,
															state_errors.back());
				}
			}
			break;
		}
	}

	if (!scope_history->is_drop) {
		for (int h_index = (int)scope_history->start_init_network_histories.size()-1; h_index >= 0; h_index--) {
			if (scope_history->start_init_network_histories[h_index] != NULL) {
				scope->start_init_networks[h_index]->load(scope_history->start_init_network_histories[h_index]);
				scope->start_init_networks[h_index]->backprop(state_errors.back());
			}
		}

		scope->start_obs_network->load(scope_history->start_obs_network_history);
		scope->start_obs_network->backprop(state_errors.back());
	}
}

void update_helper(ScopeHistory* scope_history,
				   SolutionWrapper* wrapper) {
	Scope* scope = scope_history->scope;

	if (!scope_history->is_drop) {
		for (int n_index = 0; n_index < (int)scope_history->start_init_network_histories.size(); n_index++) {
			if (scope_history->start_init_network_histories[n_index] != NULL) {
				if (scope->start_init_networks[n_index]->last_update_iter != wrapper->iters_since_update) {
					scope->start_init_networks[n_index]->update();

					scope->start_init_networks[n_index]->last_update_iter = wrapper->iters_since_update;
				}
			}
		}

		if (scope->start_obs_network->last_update_iter != wrapper->iters_since_update) {
			scope->start_obs_network->update();

			scope->start_obs_network->last_update_iter = wrapper->iters_since_update;
		}
	}

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				ActionNode* action_node = (ActionNode*)node;

				if (action_node->action_network->last_update_iter != wrapper->iters_since_update) {
					action_node->action_network->update();

					action_node->action_network->last_update_iter = wrapper->iters_since_update;
				}

				if (action_node->obs_network->last_update_iter != wrapper->iters_since_update) {
					action_node->obs_network->update();

					action_node->obs_network->last_update_iter = wrapper->iters_since_update;
				}

				for (int n_index = 0; n_index < (int)action_node_history->init_network_histories.size(); n_index++) {
					if (action_node_history->init_network_histories[n_index] != NULL) {
						if (action_node->init_networks[n_index]->last_update_iter != wrapper->iters_since_update) {
							action_node->init_networks[n_index]->update();

							action_node->init_networks[n_index]->last_update_iter = wrapper->iters_since_update;
						}
					}
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				ScopeNode* scope_node = (ScopeNode*)node;

				if (!scope_node_history->in_is_drop) {
					if (scope_node->in_network->last_update_iter != wrapper->iters_since_update) {
						scope_node->in_network->update();

						scope_node->in_network->last_update_iter = wrapper->iters_since_update;
					}
				}

				update_helper(scope_node_history->scope_history,
							  wrapper);

				if (!scope_node_history->out_is_drop) {
					if (scope_node->out_network->last_update_iter != wrapper->iters_since_update) {
						scope_node->out_network->update();

						scope_node->out_network->last_update_iter = wrapper->iters_since_update;
					}
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
				BranchNode* branch_node = (BranchNode*)node;
				if (branch_node_history->is_branch) {
					if (branch_node->branch_network->last_update_iter != wrapper->iters_since_update) {
						branch_node->branch_network->update();

						branch_node->branch_network->last_update_iter = wrapper->iters_since_update;
					}
				} else {
					if (branch_node->original_network->last_update_iter != wrapper->iters_since_update) {
						branch_node->original_network->update();

						branch_node->original_network->last_update_iter = wrapper->iters_since_update;
					}
				}
			}
			break;
		}
	}

	if (scope_history->end_score_network_history != NULL) {
		if (scope->end_score_network->last_update_iter != wrapper->iters_since_update) {
			scope->end_score_network->update();

			scope->end_score_network->last_update_iter = wrapper->iters_since_update;
		}
	}
}

void update_helper(double target_val,
				   SolutionWrapper* wrapper) {
	if (wrapper->run_type != RUN_TYPE_EXPLORE) {
		vector<Eigen::VectorXf> state_errors;

		state_errors.push_back(Eigen::VectorXf());
		state_errors.back().resize(wrapper->solution->starting_scope->num_states);
		state_errors.back().setConstant(0.0);

		backprop_helper(wrapper->scope_histories[0],
						state_errors,
						target_val,
						wrapper);

		update_helper(wrapper->scope_histories[0],
					  wrapper);
	}

	if (wrapper->run_type == RUN_TYPE_EXISTING) {
		for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
			Scope* scope = wrapper->solution->scopes[s_index];
			for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
					it != scope->nodes.end(); it++) {
				switch (it->second->type) {
				case NODE_TYPE_NOOP:
					{
						NoopNode* noop_node = (NoopNode*)it->second;
						noop_node->average_instances_per_run = 0.999*noop_node->average_instances_per_run + 0.001*noop_node->curr_num_instances;
						if (noop_node->curr_num_instances > 0) {
							noop_node->average_instances_per_hit = 0.999*noop_node->average_instances_per_hit + 0.001*noop_node->curr_num_instances;

							noop_node->curr_num_instances = 0;
						}
					}
					break;
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)it->second;
						if (!action_node->is_generic) {
							action_node->average_instances_per_run = 0.999*action_node->average_instances_per_run + 0.001*action_node->curr_num_instances;
							if (action_node->curr_num_instances > 0) {
								action_node->average_instances_per_hit = 0.999*action_node->average_instances_per_hit + 0.001*action_node->curr_num_instances;

								action_node->curr_num_instances = 0;
							}
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)it->second;
						scope_node->average_instances_per_run = 0.999*scope_node->average_instances_per_run + 0.001*scope_node->curr_num_instances;
						if (scope_node->curr_num_instances > 0) {
							scope_node->average_instances_per_hit = 0.999*scope_node->average_instances_per_hit + 0.001*scope_node->curr_num_instances;

							scope_node->curr_num_instances = 0;
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)it->second;
						branch_node->original_average_instances_per_run = 0.999*branch_node->original_average_instances_per_run + 0.001*branch_node->original_curr_num_instances;
						if (branch_node->original_curr_num_instances > 0) {
							branch_node->original_average_instances_per_hit = 0.999*branch_node->original_average_instances_per_hit + 0.001*branch_node->original_curr_num_instances;

							branch_node->original_curr_num_instances = 0;
						}
						branch_node->branch_average_instances_per_run = 0.999*branch_node->branch_average_instances_per_run + 0.001*branch_node->branch_curr_num_instances;
						if (branch_node->branch_curr_num_instances > 0) {
							branch_node->branch_average_instances_per_hit = 0.999*branch_node->branch_average_instances_per_hit + 0.001*branch_node->branch_curr_num_instances;

							branch_node->branch_curr_num_instances = 0;
						}
					}
					break;
				}
			}
		}
	}
}
