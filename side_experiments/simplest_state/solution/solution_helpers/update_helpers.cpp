#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "negate_network.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

// const double SOLUTION_TARGET_MAX_UPDATE = 0.002;
const double SOLUTION_TARGET_MAX_UPDATE = 0.00002;
/**
 * - actually more unstable when this reduces
 *   - so not actually about stability
 *     - more about quickly adjusting to issues
 */

#if defined(MDEBUG) && MDEBUG
const int ITERS_PER_RAMP = 2;
#else
const int ITERS_PER_RAMP = 4000;
#endif /* MDEBUG */

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
		// if (wrapper->iters_since_update < ONLY_UPDATE_CONSTANT_NUM_ITERS) {
		if (false) {
			for (int h_index = (int)wrapper->network_histories.size()-1; h_index >= 0; h_index--) {
				switch (wrapper->network_histories[h_index]->network->type) {
				case NETWORK_TYPE_SCORE:
					{
						ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->network_histories[h_index];
						ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
						score_network->load(score_network_history);
						score_network->update_constant(target_val);
					}
					break;
				}
			}
		} else {
			vector<double> state_errors(wrapper->solution->num_states, 0.0);
			double max_diff = 0.0;
			double max_state = 0.0;
			double max_error = 0.0;
			for (int h_index = (int)wrapper->network_histories.size()-1; h_index >= 0; h_index--) {
				switch (wrapper->network_histories[h_index]->network->type) {
				case NETWORK_TYPE_SCORE:
					{
						ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->network_histories[h_index];
						ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
						score_network->load(score_network_history);

						double diff = abs(target_val - score_network->output->acti_vals(0));
						if (diff > max_diff) {
							max_diff = diff;
						}

						for (int i_index = 0; i_index < (int)score_network->init_states.size(); i_index++) {
							int state = score_network->init_states[i_index];
							wrapper->solution->state_means[state] = 0.99999*wrapper->solution->state_means[state]
								+ 0.00001*score_network->state_input->acti_vals(i_index);
							double curr_diff = abs(score_network->state_input->acti_vals(i_index) - wrapper->solution->state_means[state]);
							wrapper->solution->state_diffs[state] = 0.99999*wrapper->solution->state_diffs[state] + 0.00001*curr_diff;
						}

						score_network->backprop(target_val,
												state_errors);
					}
					break;
				// case NETWORK_TYPE_INIT:
				// 	{
				// 		InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->network_histories[h_index];
				// 		InitNetwork* init_network = (InitNetwork*)init_network_history->network;
				// 		init_network->load(init_network_history);
				// 		init_network->backprop(state_errors);
				// 	}
				// 	break;
				// case NETWORK_TYPE_NEGATE:
				// 	{
				// 		NegateNetworkHistory* negate_network_history = (NegateNetworkHistory*)wrapper->network_histories[h_index];
				// 		NegateNetwork* negate_network = (NegateNetwork*)negate_network_history->network;
				// 		negate_network->load(negate_network_history);

				// 		double state_val = abs(negate_network->state_input);
				// 		if (state_val > max_state) {
				// 			max_state = state_val;
				// 		}

				// 		double error = abs(state_errors[negate_network->state]);
				// 		if (error > max_error) {
				// 			max_error = error;
				// 		}

				// 		negate_network->backprop(state_errors);
				// 	}
				// 	break;
				}
			}

			double max_state_val = 0.0;
			for (int s_index = 0; s_index < (int)wrapper->state.size(); s_index++) {
				double state_size = abs(wrapper->state[s_index]);
				if (state_size > max_state_val) {
					max_state_val = state_size;
				}
			}
			max_state_val = max(1.0, max(max_state, max_state_val));
			double max_state_error = 0.0;
			for (int e_index = 0; e_index < (int)state_errors.size(); e_index++) {
				double error_size = abs(state_errors[e_index]);
				if (error_size > max_state_error) {
					max_state_error = error_size;
				}
			}
			max_state_error = max(1.0, max(max_diff, max(max_error, max_state_error)));
			double max_update = max_state_val * max_state_error;
			if (max_update != 0.0) {
				wrapper->solution->average_max_update = 0.99999*wrapper->solution->average_max_update + 0.00001*max_update;
				double learning_rate = (0.3*SOLUTION_TARGET_MAX_UPDATE)/wrapper->solution->average_max_update;
				if (learning_rate*max_update > SOLUTION_TARGET_MAX_UPDATE) {
					learning_rate = SOLUTION_TARGET_MAX_UPDATE/max_update;
				}
				for (int h_index = (int)wrapper->network_histories.size()-1; h_index >= 0; h_index--) {
					switch (wrapper->network_histories[h_index]->network->type) {
					case NETWORK_TYPE_SCORE:
						{
							ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->network_histories[h_index];
							ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
							score_network->update_weights(learning_rate);
						}
						break;
					// case NETWORK_TYPE_INIT:
					// 	{
					// 		InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->network_histories[h_index];
					// 		InitNetwork* init_network = (InitNetwork*)init_network_history->network;
					// 		// init_network->update_weights(learning_rate);
					// 		init_network->update_weights(0.01*learning_rate);
					// 	}
					// 	break;
					// // case NETWORK_TYPE_NEGATE:
					// // 	{
					// // 		NegateNetworkHistory* negate_network_history = (NegateNetworkHistory*)wrapper->network_histories[h_index];
					// // 		NegateNetwork* negate_network = (NegateNetwork*)negate_network_history->network;
					// // 		negate_network->update_weights(learning_rate);
					// // 	}
					// // 	break;
					}
				}
			}
		}
		for (int h_index = (int)wrapper->network_histories.size()-1; h_index >= 0; h_index--) {
			delete wrapper->network_histories[h_index];
		}
		wrapper->network_histories.clear();
	}

	// if (wrapper->partial_state.size() > 0) {
	// 	vector<double> state_errors(wrapper->solution->num_states, 0.0);
	// 	double max_diff = 0.0;
	// 	double max_state = 0.0;
	// 	double max_error = 0.0;
	// 	for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
	// 		switch (wrapper->partial_network_histories[h_index]->network->type) {
	// 		case NETWORK_TYPE_SCORE:
	// 			{
	// 				ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->partial_network_histories[h_index];
	// 				ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
	// 				score_network->load(score_network_history);

	// 				double diff = abs(target_val - score_network->output->acti_vals(0));
	// 				if (diff > max_diff) {
	// 					max_diff = diff;
	// 				}

	// 				score_network->backprop(target_val,
	// 										state_errors);
	// 			}
	// 			break;
	// 		case NETWORK_TYPE_INIT:
	// 			{
	// 				InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->partial_network_histories[h_index];
	// 				InitNetwork* init_network = (InitNetwork*)init_network_history->network;
	// 				init_network->load(init_network_history);
	// 				init_network->backprop(state_errors);
	// 			}
	// 			break;
	// 		case NETWORK_TYPE_NEGATE:
	// 			{
	// 				NegateNetworkHistory* negate_network_history = (NegateNetworkHistory*)wrapper->partial_network_histories[h_index];
	// 				NegateNetwork* negate_network = (NegateNetwork*)negate_network_history->network;
	// 				negate_network->load(negate_network_history);

	// 				double state_val = abs(negate_network->state_input);
	// 				if (state_val > max_state) {
	// 					max_state = state_val;
	// 				}

	// 				double error = abs(state_errors[negate_network->state]);
	// 				if (error > max_error) {
	// 					max_error = error;
	// 				}

	// 				negate_network->backprop(state_errors);
	// 			}
	// 			break;
	// 		}
	// 	}

	// 	double max_state_val = 0.0;
	// 	for (int s_index = 0; s_index < (int)wrapper->partial_state.size(); s_index++) {
	// 		double state_size = abs(wrapper->partial_state[s_index]);
	// 		if (state_size > max_state_val) {
	// 			max_state_val = state_size;
	// 		}
	// 	}
	// 	max_state_val = max(1.0, max(max_state, max_state_val));
	// 	double max_state_error = 0.0;
	// 	for (int e_index = 0; e_index < (int)state_errors.size(); e_index++) {
	// 		double error_size = abs(state_errors[e_index]);
	// 		if (error_size > max_state_error) {
	// 			max_state_error = error_size;
	// 		}
	// 	}
	// 	// max_state_error = max(max_diff, max(max_error, max_state_error));
	// 	max_state_error = max(1.0, max(max_diff, max(max_error, max_state_error)));
	// 	double max_update = max_state_val * max_state_error;
	// 	if (max_update != 0.0) {
	// 		wrapper->solution->average_max_update = 0.99999*wrapper->solution->average_max_update + 0.00001*max_update;
	// 		double learning_rate = (0.3*SOLUTION_TARGET_MAX_UPDATE)/wrapper->solution->average_max_update;
	// 		if (learning_rate*max_update > SOLUTION_TARGET_MAX_UPDATE) {
	// 			learning_rate = SOLUTION_TARGET_MAX_UPDATE/max_update;
	// 		}
	// 		for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
	// 			switch (wrapper->partial_network_histories[h_index]->network->type) {
	// 			case NETWORK_TYPE_SCORE:
	// 				{
	// 					ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->partial_network_histories[h_index];
	// 					ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
	// 					score_network->update_weights(learning_rate);
	// 				}
	// 				break;
	// 			case NETWORK_TYPE_INIT:
	// 				{
	// 					InitNetworkHistory* init_network_history = (InitNetworkHistory*)wrapper->partial_network_histories[h_index];
	// 					InitNetwork* init_network = (InitNetwork*)init_network_history->network;
	// 					// init_network->update_weights(learning_rate);
	// 					init_network->update_weights(0.01*learning_rate);
	// 				}
	// 				break;
	// 			// case NETWORK_TYPE_NEGATE:
	// 			// 	{
	// 			// 		NegateNetworkHistory* negate_network_history = (NegateNetworkHistory*)wrapper->partial_network_histories[h_index];
	// 			// 		NegateNetwork* negate_network = (NegateNetwork*)negate_network_history->network;
	// 			// 		negate_network->update_weights(learning_rate);
	// 			// 	}
	// 			// 	break;
	// 			}
	// 		}
	// 	}
	// 	for (int h_index = (int)wrapper->partial_network_histories.size()-1; h_index >= 0; h_index--) {
	// 		delete wrapper->partial_network_histories[h_index];
	// 	}
	// 	wrapper->partial_network_histories.clear();
	// }

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

									// // temp
									// cout << "branch_node->ramp: " << branch_node->ramp << endl;
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

									// // temp
									// cout << "branch_node->ramp: " << branch_node->ramp << endl;
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
