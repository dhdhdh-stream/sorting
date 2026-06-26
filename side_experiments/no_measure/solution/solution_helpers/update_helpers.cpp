/**
 * TODO: need signals to update more effectively
 */

#include "solution_helpers.h"

#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int RAMP_UPDATE_MIN_SAMPLES = 1;
const int RAMP_UPDATE_NUM_TRAIN = 2;
const int ITERS_PER_RAMP = 2;
#else
const int RAMP_UPDATE_MIN_SAMPLES = 5;
const int RAMP_UPDATE_NUM_TRAIN = 10;
const int ITERS_PER_RAMP = 2000;
#endif /* MDEBUG */

void update_helper(double target_val,
				   SolutionWrapper* wrapper) {
	for (map<BranchNode*, pair<int,pair<bool,vector<double>>>>::iterator it = wrapper->branch_node_samples.begin();
			it != wrapper->branch_node_samples.end(); it++) {
		if (it->first->ramp >= RAMP_NUM_GEARS) {
			if (it->second.second.first) {
				it->first->branch_network->activate(it->second.second.second);
				double error = target_val - it->first->branch_network->output->acti_vals[0];
				it->first->branch_network->backprop(error);

				it->first->consec_original = 0;
				it->first->consec_branch++;
			} else {
				it->first->original_network->activate(it->second.second.second);
				double error = target_val - it->first->original_network->output->acti_vals[0];
				it->first->original_network->backprop(error);

				it->first->consec_original++;
				it->first->consec_branch = 0;
			}
		} else {
			if (it->second.second.first) {
				if (it->first->branch_obs_history.size() < RAMP_HISTORY_NUM_SAVE) {
					it->first->branch_obs_history.push_back(it->second.second.second);
					it->first->branch_target_val_history.push_back(target_val);
				} else {
					it->first->branch_obs_history[it->first->branch_index] = it->second.second.second;
					it->first->branch_target_val_history[it->first->branch_index] = target_val;
				}
				it->first->branch_index++;
				if (it->first->branch_index >= RAMP_HISTORY_NUM_SAVE) {
					it->first->branch_index = 0;
				}

				if (it->first->branch_obs_history.size() >= RAMP_UPDATE_MIN_SAMPLES) {
					uniform_int_distribution<int> distribution(0, it->first->branch_obs_history.size()-1);
					for (int iter_index = 0; iter_index < RAMP_UPDATE_NUM_TRAIN; iter_index++) {
						int index = distribution(generator);
						it->first->branch_network->activate(it->first->branch_obs_history[index]);
						double error = it->first->branch_target_val_history[index] - it->first->branch_network->output->acti_vals[0];
						it->first->branch_network->backprop(error);
					}
				}

				it->first->consec_original = 0;
				it->first->consec_branch++;
			} else {
				if (it->first->original_obs_history.size() < RAMP_HISTORY_NUM_SAVE) {
					it->first->original_obs_history.push_back(it->second.second.second);
					it->first->original_target_val_history.push_back(target_val);
				} else {
					it->first->original_obs_history[it->first->original_index] = it->second.second.second;
					it->first->original_target_val_history[it->first->original_index] = target_val;
				}
				it->first->original_index++;
				if (it->first->original_index >= RAMP_HISTORY_NUM_SAVE) {
					it->first->original_index = 0;
				}

				if (it->first->original_obs_history.size() >= RAMP_UPDATE_MIN_SAMPLES) {
					uniform_int_distribution<int> distribution(0, it->first->original_obs_history.size()-1);
					for (int iter_index = 0; iter_index < RAMP_UPDATE_NUM_TRAIN; iter_index++) {
						int index = distribution(generator);
						it->first->original_network->activate(it->first->original_obs_history[index]);
						double error = it->first->original_target_val_history[index] - it->first->original_network->output->acti_vals[0];
						it->first->original_network->backprop(error);
					}
				}

				uniform_int_distribution<int> on_distribution(0, RAMP_NUM_GEARS);
				if (it->first->ramp >= on_distribution(generator)) {
					it->first->consec_original++;
				}
				it->first->consec_branch = 0;
			}

			it->first->ramp_iter++;
			if (it->first->ramp_iter % ITERS_PER_RAMP == 0) {
				it->first->ramp++;

				// // temp
				// cout << "it->first->ramp: " << it->first->ramp << endl;
			}
		}
	}

	wrapper->branch_node_samples.clear();
}
