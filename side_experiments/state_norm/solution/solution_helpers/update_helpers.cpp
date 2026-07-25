#include "solution_helpers.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "negate_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const double SOLUTION_TARGET_MAX_UPDATE = 0.01;

#if defined(MDEBUG) && MDEBUG
const int ITERS_PER_RAMP = 2;
#else
const int ITERS_PER_RAMP = 4000;
#endif /* MDEBUG */

const int LARGEST_MAX_UPDATES_NUM_TRACK = 10;

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
				action_node->curr_num_instances++;
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

/**
 * - for some reason, best to update each network individually as frequently as possible(?)
 *   - vs. updating all networks in a balanced way
 */
void update_helper(double target_val,
				   SolutionWrapper* wrapper) {
	if (wrapper->run_type != RUN_TYPE_EXPLORE) {
		Eigen::VectorXf state_errors;
		state_errors.resize(wrapper->solution->num_states);
		state_errors.setConstant(0.0);
		for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
			switch (wrapper->partial_network_histories[h_index]->network->type) {
			case NETWORK_TYPE_OBS:
				{
					ObsNetworkHistory* obs_network_history = (ObsNetworkHistory*)wrapper->partial_network_histories[h_index];
					ObsNetwork* obs_network = (ObsNetwork*)obs_network_history->network;
					obs_network->load(obs_network_history);
					obs_network->backprop(state_errors);
				}
				break;
			case NETWORK_TYPE_SCORE:
				{
					ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->partial_network_histories[h_index];
					ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
					score_network->load(score_network_history);
					score_network->backprop(target_val,
											state_errors);
				}
				break;
			case NETWORK_TYPE_ACTION:
				{
					ActionNetworkHistory* action_network_history = (ActionNetworkHistory*)wrapper->partial_network_histories[h_index];
					ActionNetwork* action_network = (ActionNetwork*)action_network_history->network;
					action_network->load(action_network_history);
					action_network->backprop(state_errors);
				}
				break;
			case NETWORK_TYPE_INIT:
				{
					InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->partial_network_histories[h_index];
					InitNetwork* init_network = (InitNetwork*)init_network_history->network;
					init_network->load(init_network_history);
					init_network->backprop(state_errors);
				}
				break;
			case NETWORK_TYPE_NEGATE:
				{
					NegateNetworkHistory* negate_network_history = (NegateNetworkHistory*)wrapper->partial_network_histories[h_index];
					NegateNetwork* negate_network = (NegateNetwork*)negate_network_history->network;
					int state_index = negate_network->state_index;
					state_errors(state_index) = 0.0;
				}
				break;
			}
		}

		double max_update_size = 0.0;
		for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
			switch (wrapper->partial_network_histories[h_index]->network->type) {
			case NETWORK_TYPE_OBS:
				{
					ObsNetworkHistory* obs_network_history = (ObsNetworkHistory*)wrapper->partial_network_histories[h_index];
					ObsNetwork* obs_network = (ObsNetwork*)obs_network_history->network;
					if (obs_network->last_get_max_update_iter != wrapper->iters_since_update) {
						obs_network->get_max_update(max_update_size);

						obs_network->last_get_max_update_iter = wrapper->iters_since_update;
					}
				}
				break;
			case NETWORK_TYPE_SCORE:
				{
					ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->partial_network_histories[h_index];
					ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
					if (score_network->last_get_max_update_iter != wrapper->iters_since_update) {
						score_network->get_max_update(max_update_size);

						score_network->last_get_max_update_iter = wrapper->iters_since_update;
					}
				}
				break;
			case NETWORK_TYPE_ACTION:
				{
					ActionNetworkHistory* action_network_history = (ActionNetworkHistory*)wrapper->partial_network_histories[h_index];
					ActionNetwork* action_network = (ActionNetwork*)action_network_history->network;
					if (action_network->last_get_max_update_iter != wrapper->iters_since_update) {
						action_network->get_max_update(max_update_size);

						action_network->last_get_max_update_iter = wrapper->iters_since_update;
					}
				}
				break;
			case NETWORK_TYPE_INIT:
				{
					InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->partial_network_histories[h_index];
					InitNetwork* init_network = (InitNetwork*)init_network_history->network;
					if (init_network->last_get_max_update_iter != wrapper->iters_since_update) {
						init_network->get_max_update(max_update_size);

						init_network->last_get_max_update_iter = wrapper->iters_since_update;
					}
				}
				break;
			}
		}

		wrapper->solution->largest_max_updates.push_back(max_update_size);
		sort(wrapper->solution->largest_max_updates.begin(), wrapper->solution->largest_max_updates.end());
		if (wrapper->solution->largest_max_updates.size() > LARGEST_MAX_UPDATES_NUM_TRACK) {
			wrapper->solution->largest_max_updates.erase(wrapper->solution->largest_max_updates.begin());
		}

		// // temp
		// if (max_update_size > 1000.0) {
		// 	print_run_helper(wrapper->scope_histories[0]);
		// 	print_state_helper(wrapper);
		// 	print_error_helper(target_val,
		// 					   wrapper);

		// 	throw invalid_argument("max_update_size > 1000.0");
		// }

		wrapper->solution->average_max_update = 0.99999*wrapper->solution->average_max_update + 0.00001*max_update_size;
		if (max_update_size != 0.0) {
			double learning_rate = (0.3*SOLUTION_TARGET_MAX_UPDATE)/wrapper->solution->average_max_update;
			if (learning_rate*max_update_size > SOLUTION_TARGET_MAX_UPDATE) {
				learning_rate = SOLUTION_TARGET_MAX_UPDATE/max_update_size;
			}
			for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
				switch (wrapper->partial_network_histories[h_index]->network->type) {
				case NETWORK_TYPE_OBS:
					{
						ObsNetworkHistory* obs_network_history = (ObsNetworkHistory*)wrapper->partial_network_histories[h_index];
						ObsNetwork* obs_network = (ObsNetwork*)obs_network_history->network;
						if (obs_network->last_update_weights_iter != wrapper->iters_since_update) {
							obs_network->update_weights(learning_rate);

							obs_network->last_update_weights_iter = wrapper->iters_since_update;
						}
					}
					break;
				case NETWORK_TYPE_SCORE:
					{
						ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->partial_network_histories[h_index];
						ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
						if (score_network->last_update_weights_iter != wrapper->iters_since_update) {
							score_network->update_weights(learning_rate);

							score_network->last_update_weights_iter = wrapper->iters_since_update;
						}
					}
					break;
				case NETWORK_TYPE_ACTION:
					{
						ActionNetworkHistory* action_network_history = (ActionNetworkHistory*)wrapper->partial_network_histories[h_index];
						ActionNetwork* action_network = (ActionNetwork*)action_network_history->network;
						if (action_network->last_update_weights_iter != wrapper->iters_since_update) {
							action_network->update_weights(learning_rate);

							action_network->last_update_weights_iter = wrapper->iters_since_update;
						}
					}
					break;
				case NETWORK_TYPE_INIT:
					{
						InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->partial_network_histories[h_index];
						InitNetwork* init_network = (InitNetwork*)init_network_history->network;
						if (init_network->last_update_weights_iter != wrapper->iters_since_update) {
							init_network->update_weights(learning_rate);

							init_network->last_update_weights_iter = wrapper->iters_since_update;
						}
					}
					break;
				}
			}
		}
	}

	for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
		delete wrapper->partial_network_histories[h_index];
	}
	wrapper->partial_network_histories.clear();

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
						action_node->average_instances_per_run = 0.999*action_node->average_instances_per_run + 0.001*action_node->curr_num_instances;
						if (action_node->curr_num_instances > 0) {
							action_node->average_instances_per_hit = 0.999*action_node->average_instances_per_hit + 0.001*action_node->curr_num_instances;

							action_node->curr_num_instances = 0;
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

							if (branch_node->ramp < branch_node->ramp_num_gears) {
								branch_node->ramp_iter++;
								if (branch_node->ramp_iter >= ITERS_PER_RAMP) {
									branch_node->ramp++;
									branch_node->ramp_iter = 0;
								}
							}

							branch_node->original_curr_num_instances = 0;
						}
						branch_node->branch_average_instances_per_run = 0.999*branch_node->branch_average_instances_per_run + 0.001*branch_node->branch_curr_num_instances;
						if (branch_node->branch_curr_num_instances > 0) {
							branch_node->branch_average_instances_per_hit = 0.999*branch_node->branch_average_instances_per_hit + 0.001*branch_node->branch_curr_num_instances;

							if (branch_node->ramp < branch_node->ramp_num_gears) {
								branch_node->ramp_iter++;
								if (branch_node->ramp_iter >= ITERS_PER_RAMP) {
									branch_node->ramp++;
									branch_node->ramp_iter = 0;
								}
							}

							branch_node->branch_curr_num_instances = 0;
						}
					}
					break;
				}
			}
		}
	}
}
