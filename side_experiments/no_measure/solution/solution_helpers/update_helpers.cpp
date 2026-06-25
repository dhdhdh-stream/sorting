#include "solution_helpers.h"

#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

const int RAMP_UPDATE_MIN_SAMPLES = 10;
#if defined(MDEBUG) && MDEBUG
const int RAMP_UPDATE_NUM_TRAIN = 2;
const int ITERS_PER_RAMP = 2;
#else
const int RAMP_UPDATE_NUM_TRAIN = 100;
const int ITERS_PER_RAMP = 100;
#endif /* MDEBUG */

void update_helper(ScopeHistory* scope_history,
				   double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				update_helper(scope_node_history->scope_history,
							  target_val);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;

				if (branch_node->ramp >= RAMP_NUM_GEARS) {
					if (branch_node_history->is_branch) {
						branch_node->branch_network->activate(branch_node_history->obs);
						double error = target_val - branch_node->branch_network->output->acti_vals[0];
						branch_node->branch_network->backprop(error);

						branch_node->consec_original = 0;
						branch_node->consec_branch++;
					} else {
						branch_node->original_network->activate(branch_node_history->obs);
						double error = target_val - branch_node->original_network->output->acti_vals[0];
						branch_node->original_network->backprop(error);

						branch_node->consec_original++;
						branch_node->consec_branch = 0;
					}
				} else {
					if (branch_node_history->is_branch) {
						if (branch_node->branch_obs_history.size() < RAMP_HISTORY_NUM_SAVE) {
							branch_node->branch_obs_history.push_back(branch_node_history->obs);
							branch_node->branch_target_val_history.push_back(target_val);
						} else {
							branch_node->branch_obs_history[branch_node->branch_index] = branch_node_history->obs;
							branch_node->branch_target_val_history[branch_node->branch_index] = target_val;
						}
						branch_node->branch_index++;
						if (branch_node->branch_index >= RAMP_HISTORY_NUM_SAVE) {
							branch_node->branch_index = 0;
						}

						if (branch_node->branch_obs_history.size() >= RAMP_UPDATE_MIN_SAMPLES) {
							uniform_int_distribution<int> distribution(0, branch_node->branch_obs_history.size()-1);
							for (int iter_index = 0; iter_index < RAMP_UPDATE_NUM_TRAIN; iter_index++) {
								int index = distribution(generator);
								branch_node->branch_network->activate(branch_node->branch_obs_history[index]);
								double error = branch_node->branch_target_val_history[index] - branch_node->branch_network->output->acti_vals[0];
								branch_node->branch_network->backprop(error);
							}
						}

						branch_node->ramp_iter++;
						if (branch_node->ramp_iter >= ITERS_PER_RAMP) {
							branch_node->ramp++;
							branch_node->ramp_iter = 0;

							// // temp
							// cout << "branch_node->ramp: " << branch_node->ramp << endl;

							if (branch_node->ramp >= RAMP_NUM_GEARS) {
								branch_node->original_obs_history.clear();
								branch_node->original_target_val_history.clear();
								branch_node->original_index = 0;
								branch_node->branch_obs_history.clear();
								branch_node->branch_target_val_history.clear();
								branch_node->branch_index = 0;
							}
						}
					} else {
						if (branch_node->original_obs_history.size() < RAMP_HISTORY_NUM_SAVE) {
							branch_node->original_obs_history.push_back(branch_node_history->obs);
							branch_node->original_target_val_history.push_back(target_val);
						} else {
							branch_node->original_obs_history[branch_node->original_index] = branch_node_history->obs;
							branch_node->original_target_val_history[branch_node->original_index] = target_val;
						}
						branch_node->original_index++;
						if (branch_node->original_index >= RAMP_HISTORY_NUM_SAVE) {
							branch_node->original_index = 0;
						}

						if (branch_node->original_obs_history.size() >= RAMP_UPDATE_MIN_SAMPLES) {
							uniform_int_distribution<int> distribution(0, branch_node->original_obs_history.size()-1);
							for (int iter_index = 0; iter_index < RAMP_UPDATE_NUM_TRAIN; iter_index++) {
								int index = distribution(generator);
								branch_node->original_network->activate(branch_node->original_obs_history[index]);
								double error = branch_node->original_target_val_history[index] - branch_node->original_network->output->acti_vals[0];
								branch_node->original_network->backprop(error);
							}
						}
					}
				}
			}
			break;
		}
	}
}
