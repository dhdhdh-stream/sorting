/**
 * - even updating 1 iter per run makes big difference
 * 
 * - only use 1 instance per run to avoid bias
 * 
 * - try to have enough update iters to keep matched to current solution
 *   - so can keep reinforcing features instead of chasing new average
 *   - so also update original and branch at the same speed
 * 
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
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int ITERS_PER_RAMP = 2;
#else
const int ITERS_PER_RAMP = 1000;
#endif /* MDEBUG */

const double TARGET_ITERS_PER_RUN = 1.0;
const double RAMP_TARGET_ITERS_PER_RUN = 10.0;
#if defined(MDEBUG) && MDEBUG
const int MAX_ITERS_PER_RUN = 1;
const int RAMP_MAX_ITERS_PER_RUN = 1;
#else
const int MAX_ITERS_PER_RUN = 10;
const int RAMP_MAX_ITERS_PER_RUN = 100;
#endif /* MDEBUG */

void update_helper(double target_val,
				   SolutionWrapper* wrapper) {
	for (map<BranchNode*, pair<int,vector<double>>>::iterator it = wrapper->original_samples.begin();
			it != wrapper->original_samples.end(); it++) {
		if (it->first->original_obs_history.size() < RAMP_HISTORY_NUM_SAVE) {
			it->first->original_obs_history.push_back(it->second.second);
			it->first->original_target_val_history.push_back(target_val);
		} else {
			it->first->original_obs_history[it->first->original_index] = it->second.second;
			it->first->original_target_val_history[it->first->original_index] = target_val;
		}
		it->first->original_index++;
		if (it->first->original_index >= RAMP_HISTORY_NUM_SAVE) {
			it->first->original_index = 0;
		}

		int num_iters;
		if (it->first->ramp >= RAMP_NUM_GEARS) {
			if (it->first->original_hits_per_run <= it->first->branch_hits_per_run) {
				num_iters = TARGET_ITERS_PER_RUN / it->first->original_hits_per_run;
			} else {
				double ratio = it->first->branch_hits_per_run / it->first->original_hits_per_run;
				num_iters = ratio * TARGET_ITERS_PER_RUN / it->first->original_hits_per_run;
			}
			if (num_iters > MAX_ITERS_PER_RUN) {
				num_iters = MAX_ITERS_PER_RUN;
			}

			it->first->consec_original++;
			it->first->consec_branch = 0;
		} else {
			if (it->first->original_hits_per_run <= it->first->branch_hits_per_run) {
				num_iters = RAMP_TARGET_ITERS_PER_RUN / it->first->original_hits_per_run;
			} else {
				double ratio = it->first->branch_hits_per_run / it->first->original_hits_per_run;
				num_iters = ratio * RAMP_TARGET_ITERS_PER_RUN / it->first->original_hits_per_run;
			}
			if (num_iters > RAMP_MAX_ITERS_PER_RUN) {
				num_iters = RAMP_MAX_ITERS_PER_RUN;
			}

			uniform_int_distribution<int> on_distribution(0, RAMP_NUM_GEARS);
			if (it->first->ramp >= on_distribution(generator)) {
				it->first->consec_original++;
			}
			it->first->consec_branch = 0;
		}

		if (num_iters >= (int)it->first->original_obs_history.size()) {
			num_iters = (int)it->first->original_obs_history.size();
		}

		uniform_int_distribution<int> distribution(0, it->first->original_obs_history.size()-1);
		for (int iter_index = 0; iter_index < num_iters; iter_index++) {
			int index = distribution(generator);
			it->first->original_network->activate(it->first->original_obs_history[index]);
			double error = it->first->original_target_val_history[index] - it->first->original_network->output->acti_vals[0];
			it->first->original_network->backprop(error);
		}

		it->first->curr_original_hit = true;
	}

	for (map<BranchNode*, pair<int,vector<double>>>::iterator it = wrapper->branch_samples.begin();
			it != wrapper->branch_samples.end(); it++) {
		if (it->first->branch_obs_history.size() < RAMP_HISTORY_NUM_SAVE) {
			it->first->branch_obs_history.push_back(it->second.second);
			it->first->branch_target_val_history.push_back(target_val);
		} else {
			it->first->branch_obs_history[it->first->branch_index] = it->second.second;
			it->first->branch_target_val_history[it->first->branch_index] = target_val;
		}
		it->first->branch_index++;
		if (it->first->branch_index >= RAMP_HISTORY_NUM_SAVE) {
			it->first->branch_index = 0;
		}

		int num_iters;
		if (it->first->ramp >= RAMP_NUM_GEARS) {
			if (it->first->branch_hits_per_run <= it->first->original_hits_per_run) {
				num_iters = TARGET_ITERS_PER_RUN / it->first->branch_hits_per_run;
			} else {
				double ratio = it->first->original_hits_per_run / it->first->branch_hits_per_run;
				num_iters = ratio * TARGET_ITERS_PER_RUN / it->first->branch_hits_per_run;
			}
			if (num_iters > MAX_ITERS_PER_RUN) {
				num_iters = MAX_ITERS_PER_RUN;
			}

			it->first->consec_original = 0;
			it->first->consec_branch++;
		} else {
			if (it->first->branch_hits_per_run <= it->first->original_hits_per_run) {
				num_iters = RAMP_TARGET_ITERS_PER_RUN / it->first->branch_hits_per_run;
			} else {
				double ratio = it->first->original_hits_per_run / it->first->branch_hits_per_run;
				num_iters = ratio * RAMP_TARGET_ITERS_PER_RUN / it->first->branch_hits_per_run;
			}
			if (num_iters > RAMP_MAX_ITERS_PER_RUN) {
				num_iters = RAMP_MAX_ITERS_PER_RUN;
			}

			it->first->consec_original = 0;
			it->first->consec_branch++;

			it->first->ramp_iter++;
			if (it->first->ramp_iter % ITERS_PER_RAMP == 0) {
				it->first->ramp++;
			}
		}

		if (num_iters >= (int)it->first->branch_obs_history.size()) {
			num_iters = (int)it->first->branch_obs_history.size();
		}

		uniform_int_distribution<int> distribution(0, it->first->branch_obs_history.size()-1);
		for (int iter_index = 0; iter_index < num_iters; iter_index++) {
			int index = distribution(generator);
			it->first->branch_network->activate(it->first->branch_obs_history[index]);
			double error = it->first->branch_target_val_history[index] - it->first->branch_network->output->acti_vals[0];
			it->first->branch_network->backprop(error);
		}

		it->first->curr_branch_hit = true;
	}

	wrapper->original_samples.clear();
	wrapper->branch_samples.clear();

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_BRANCH) {
				BranchNode* branch_node = (BranchNode*)it->second;
				if (branch_node->curr_original_hit) {
					branch_node->original_hits_per_run = 0.99*branch_node->original_hits_per_run + 0.01;
					branch_node->curr_original_hit = false;
				} else {
					branch_node->original_hits_per_run = 0.99*branch_node->original_hits_per_run;
				}
				if (branch_node->curr_branch_hit) {
					branch_node->branch_hits_per_run = 0.99*branch_node->branch_hits_per_run + 0.01;
					branch_node->curr_branch_hit = false;
				} else {
					branch_node->branch_hits_per_run = 0.99*branch_node->branch_hits_per_run;
				}
			}
		}
	}
}
