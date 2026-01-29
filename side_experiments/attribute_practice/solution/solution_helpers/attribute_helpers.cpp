#include "solution_helpers.h"

#include "action_node.h"
#include "constants.h"
#include "decision_tree.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int LONG_NUM_SAMPLES = 10;
const int UPDATE_ITERS = 10;
#else
const int LONG_NUM_SAMPLES = 1000;
const int UPDATE_ITERS = 100;
#endif /* MDEBUG */

const int INIT_NUM_CYCLES = 10;

const int EXISTING_MAX_WEIGHT = 9;

// void train_helper(Scope* scope) {
// 	if (scope->pre_network == NULL) {
// 		Network* pre_network = new Network(scope->pre_obs[0].size());
// 		scope->pre_network = pre_network;

// 		for (int c_index = 0; c_index < INIT_NUM_CYCLES; c_index++) {
// 			for (int h_index = 0; h_index < LONG_NUM_SAMPLES; h_index++) {
// 				pre_network->activate(scope->pre_obs[h_index]);

// 				double error = scope->pre_targets[h_index] - pre_network->output->acti_vals[0];

// 				pre_network->backprop(error);
// 			}
// 		}

// 		for (int h_index = 0; h_index < LONG_NUM_SAMPLES; h_index++) {
// 			pre_network->activate(scope->pre_obs[h_index]);
// 			scope->post_targets[h_index] -= pre_network->output->acti_vals[0];
// 		}

// 		Network* post_network = new Network(scope->post_obs[0].size());
// 		scope->post_network = post_network;

// 		for (int c_index = 0; c_index < INIT_NUM_CYCLES; c_index++) {
// 			for (int h_index = 0; h_index < LONG_NUM_SAMPLES; h_index++) {
// 				post_network->activate(scope->post_obs[h_index]);

// 				double error = scope->post_targets[h_index] - post_network->output->acti_vals[0];

// 				post_network->backprop(error);
// 			}
// 		}
// 	} else {
// 		Network* pre_network = scope->pre_network;

// 		vector<double> pre_adjusted_targets(LONG_NUM_SAMPLES);
// 		for (int h_index = 0; h_index < LONG_NUM_SAMPLES; h_index++) {
// 			pre_network->activate(scope->pre_obs[h_index]);
// 			if (scope->long_iter > EXISTING_MAX_WEIGHT) {
// 				pre_adjusted_targets[h_index] = (EXISTING_MAX_WEIGHT * pre_network->output->acti_vals[0] + scope->pre_targets[h_index]) / (1 + EXISTING_MAX_WEIGHT);
// 			} else {
// 				pre_adjusted_targets[h_index] = (scope->long_iter * pre_network->output->acti_vals[0] + scope->pre_targets[h_index]) / (1 + scope->long_iter);
// 			}
// 		}

// 		for (int h_index = 0; h_index < LONG_NUM_SAMPLES; h_index++) {
// 			pre_network->activate(scope->pre_obs[h_index]);

// 			double error = pre_adjusted_targets[h_index] - pre_network->output->acti_vals[0];

// 			pre_network->backprop(error);
// 		}

// 		Network* post_network = scope->post_network;

// 		vector<double> post_adjusted_targets(LONG_NUM_SAMPLES);
// 		for (int h_index = 0; h_index < LONG_NUM_SAMPLES; h_index++) {
// 			post_network->activate(scope->post_obs[h_index]);
// 			if (scope->long_iter > EXISTING_MAX_WEIGHT) {
// 				post_adjusted_targets[h_index] = (EXISTING_MAX_WEIGHT * post_network->output->acti_vals[0] + scope->post_targets[h_index]) / (1 + EXISTING_MAX_WEIGHT);
// 			} else {
// 				post_adjusted_targets[h_index] = (scope->long_iter * post_network->output->acti_vals[0] + scope->post_targets[h_index]) / (1 + scope->long_iter);
// 			}
// 		}

// 		for (int h_index = 0; h_index < LONG_NUM_SAMPLES; h_index++) {
// 			post_network->activate(scope->post_obs[h_index]);

// 			double error = post_adjusted_targets[h_index] - post_network->output->acti_vals[0];

// 			post_network->backprop(error);
// 		}
// 	}
// }

void update_attribute(ScopeHistory* scope_history,
					  double target_val,
					  SolutionWrapper* wrapper) {
	// Scope* scope = scope_history->scope;

	// if (scope->pre_obs.size() < LONG_NUM_SAMPLES) {
	// 	scope->pre_obs.push_back(scope_history->pre_obs_history);
	// 	scope->pre_targets.push_back(target_val - scope_history->pre_impact);
	// 	scope->post_obs.push_back(scope_history->post_obs_history);
	// 	scope->post_targets.push_back(target_val - scope_history->post_impact);

	// 	if (scope->pre_obs.size() >= LONG_NUM_SAMPLES) {
	// 		train_helper(scope);

	// 		scope->long_iter++;
	// 	}
	// } else {
	// 	scope->pre_obs[scope->data_index] = scope_history->pre_obs_history;
	// 	scope->pre_targets[scope->data_index] = target_val - scope_history->pre_impact;
	// 	scope->post_obs[scope->data_index] = scope_history->post_obs_history;
	// 	scope->post_targets[scope->data_index] = target_val - scope_history->post_impact;

	// 	scope->data_index++;
	// 	if (scope->data_index % UPDATE_ITERS == 0) {
	// 		train_helper(scope);
	// 	}
	// 	if (scope->data_index >= LONG_NUM_SAMPLES) {
	// 		scope->data_index = 0;

	// 		scope->long_iter++;
	// 	}
	// }

	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->second->node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)it->second->node;
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				map<int, DecisionTree*>::iterator network_it = wrapper->solution->action_impact_networks.find(action_node->action);
				if (network_it != wrapper->solution->action_impact_networks.end()) {
					network_it->second->backprop(action_node_history->obs_history,
												 target_val - action_node_history->curr_impact);
				}
			}

			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				update_attribute(scope_node_history->scope_history,
								 target_val,
								 wrapper);
			}

			break;
		}
	}
}
