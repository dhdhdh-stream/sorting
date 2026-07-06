#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int ITERS_PER_RAMP = 2;
#else
const int ITERS_PER_RAMP = 4000;
#endif /* MDEBUG */

void update_helper(ScopeHistory* scope_history,
				   double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNodeHistory* noop_node_history = (NoopNodeHistory*)h_it->second;
				NoopNode* noop_node = (NoopNode*)noop_node_history->node;

				if (noop_node->sample_obs.size() < SAMPLES_NUM_SAVE) {
					noop_node->sample_obs.push_back(noop_node_history->obs);
					noop_node->sample_target_vals.push_back(target_val);
				} else {
					noop_node->sample_obs[noop_node->sample_index] = noop_node_history->obs;
					noop_node->sample_target_vals[noop_node->sample_index] = target_val;
				}
				noop_node->sample_index++;
				if (noop_node->sample_index >= SAMPLES_NUM_SAVE) {
					noop_node->sample_index = 0;
				}

				noop_node->network->activate(noop_node_history->obs);
				double error = target_val - noop_node->network->output->acti_vals(0);
				noop_node->network->backprop(error);

				noop_node->curr_instances_per_run++;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)h_it->second;
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				if (action_node->sample_obs.size() < SAMPLES_NUM_SAVE) {
					action_node->sample_obs.push_back(action_node_history->obs);
					action_node->sample_target_vals.push_back(target_val);
				} else {
					action_node->sample_obs[action_node->sample_index] = action_node_history->obs;
					action_node->sample_target_vals[action_node->sample_index] = target_val;
				}
				action_node->sample_index++;
				if (action_node->sample_index >= SAMPLES_NUM_SAVE) {
					action_node->sample_index = 0;
				}

				action_node->network->activate(action_node_history->obs);
				double error = target_val - action_node->network->output->acti_vals(0);
				action_node->network->backprop(error);

				action_node->curr_instances_per_run++;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				update_helper(scope_node_history->scope_history,
							  target_val);

				if (scope_node->sample_obs.size() < SAMPLES_NUM_SAVE) {
					scope_node->sample_obs.push_back(scope_node_history->obs);
					scope_node->sample_target_vals.push_back(target_val);
				} else {
					scope_node->sample_obs[scope_node->sample_index] = scope_node_history->obs;
					scope_node->sample_target_vals[scope_node->sample_index] = target_val;
				}
				scope_node->sample_index++;
				if (scope_node->sample_index >= SAMPLES_NUM_SAVE) {
					scope_node->sample_index = 0;
				}

				scope_node->network->activate(scope_node_history->obs);
				double error = target_val - scope_node->network->output->acti_vals(0);
				scope_node->network->backprop(error);

				scope_node->curr_instances_per_run++;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;

				if (branch_node_history->is_branch) {
					if (branch_node->branch_sample_obs.size() < SAMPLES_NUM_SAVE) {
						branch_node->branch_sample_obs.push_back(branch_node_history->obs);
						branch_node->branch_sample_target_vals.push_back(target_val);
					} else {
						branch_node->branch_sample_obs[branch_node->branch_sample_index] = branch_node_history->obs;
						branch_node->branch_sample_target_vals[branch_node->branch_sample_index] = target_val;
					}
					branch_node->branch_sample_index++;
					if (branch_node->branch_sample_index >= SAMPLES_NUM_SAVE) {
						branch_node->branch_sample_index = 0;
					}

					branch_node->branch_network->activate(branch_node_history->obs);
					double error = target_val - branch_node->branch_network->output->acti_vals(0);
					branch_node->branch_network->backprop(error);

					branch_node->branch_curr_instances_per_run++;
				} else {
					if (branch_node->branch_sample_obs.size() < SAMPLES_NUM_SAVE) {
						branch_node->branch_sample_obs.push_back(branch_node_history->obs);
						branch_node->branch_sample_target_vals.push_back(target_val);
					} else {
						branch_node->branch_sample_obs[branch_node->branch_sample_index] = branch_node_history->obs;
						branch_node->branch_sample_target_vals[branch_node->branch_sample_index] = target_val;
					}
					branch_node->branch_sample_index++;
					if (branch_node->branch_sample_index >= SAMPLES_NUM_SAVE) {
						branch_node->branch_sample_index = 0;
					}

					branch_node->original_network->activate(branch_node_history->obs);
					double error = target_val - branch_node->original_network->output->acti_vals(0);
					branch_node->original_network->backprop(error);

					branch_node->original_curr_instances_per_run++;
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
void update_helper(SolutionWrapper* wrapper) {
	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			switch (it->second->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)it->second;
					noop_node->average_instances_per_run = 0.999*noop_node->average_instances_per_run + 0.001*noop_node->curr_instances_per_run;
					if (noop_node->curr_instances_per_run > 0) {
						noop_node->network->update();

						noop_node->curr_instances_per_run = 0;
					}
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)it->second;
					action_node->average_instances_per_run = 0.999*action_node->average_instances_per_run + 0.001*action_node->curr_instances_per_run;
					if (action_node->curr_instances_per_run > 0) {
						action_node->network->update();

						action_node->curr_instances_per_run = 0;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)it->second;
					scope_node->average_instances_per_run = 0.999*scope_node->average_instances_per_run + 0.001*scope_node->curr_instances_per_run;
					if (scope_node->curr_instances_per_run > 0) {
						scope_node->network->update();

						scope_node->curr_instances_per_run = 0;
					}
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->original_average_instances_per_run = 0.999*branch_node->original_average_instances_per_run + 0.001*branch_node->original_curr_instances_per_run;
					if (branch_node->original_curr_instances_per_run > 0) {
						branch_node->original_network->update();

						if (branch_node->ramp < branch_node->ramp_num_gears) {
							branch_node->ramp_iter++;
							if (branch_node->ramp_iter >= ITERS_PER_RAMP) {
								branch_node->ramp++;
								branch_node->ramp_iter = 0;

								// // temp
								// cout << "branch_node->ramp: " << branch_node->ramp << endl;
							}
						}

						branch_node->original_curr_instances_per_run = 0;
					}
					branch_node->branch_average_instances_per_run = 0.999*branch_node->branch_average_instances_per_run + 0.001*branch_node->branch_curr_instances_per_run;
					if (branch_node->branch_curr_instances_per_run > 0) {
						branch_node->branch_network->update();

						if (branch_node->ramp < branch_node->ramp_num_gears) {
							branch_node->ramp_iter++;
							if (branch_node->ramp_iter >= ITERS_PER_RAMP) {
								branch_node->ramp++;
								branch_node->ramp_iter = 0;

								// // temp
								// cout << "branch_node->ramp: " << branch_node->ramp << endl;
							}
						}

						branch_node->branch_curr_instances_per_run = 0;
					}
				}
				break;
			}
		}
	}
}
