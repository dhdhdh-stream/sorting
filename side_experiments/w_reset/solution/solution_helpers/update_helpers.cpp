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

const double BASE_FACTOR = 0.001;

const int RAMP_EPOCH_SIZE = 20;
const int UPDATE_EPOCH_SIZE = 100;

void update_helper(ScopeHistory* scope_history,
				   double target_val,
				   set<BranchNode*>& hit_original,
				   set<BranchNode*>& hit_branch,
				   SolutionWrapper* wrapper) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				update_helper(scope_node_history->scope_history,
							  target_val,
							  hit_original,
							  hit_branch,
							  wrapper);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;

				if (branch_node_history->is_branch) {
					branch_node->branch_curr_num_instances++;

					double factor = BASE_FACTOR / branch_node->branch_average_instances_per_hit;
					branch_node->branch_val_average = (1.0-factor)*branch_node->branch_val_average + factor*target_val;

					branch_node->branch_network->activate(branch_node_history->obs);
					double error = (target_val - branch_node->branch_val_average) - branch_node->branch_network->output->acti_vals(0);
					branch_node->branch_network->backprop(error);

					hit_branch.insert(branch_node);
				} else {
					branch_node->original_curr_num_instances++;

					double factor = BASE_FACTOR / branch_node->original_average_instances_per_hit;
					branch_node->original_val_average = (1.0-factor)*branch_node->original_val_average + factor*target_val;

					branch_node->original_network->activate(branch_node_history->obs);
					double error = (target_val - branch_node->original_val_average) - branch_node->original_network->output->acti_vals(0);
					branch_node->original_network->backprop(error);

					hit_original.insert(branch_node);
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
void update_helper(set<BranchNode*>& hit_original,
				   set<BranchNode*>& hit_branch,
				   SolutionWrapper* wrapper) {
	for (set<BranchNode*>::iterator it = hit_original.begin();
			it != hit_original.end(); it++) {
		BranchNode* branch_node = *it;

		branch_node->original_network->epoch_iter++;
		if (branch_node->is_ramp) {
			if (branch_node->original_network->epoch_iter >= RAMP_EPOCH_SIZE) {
				branch_node->original_network->update();

				branch_node->original_network->epoch_iter = 0;
			}
		} else {
			if (branch_node->original_network->epoch_iter >= UPDATE_EPOCH_SIZE) {
				branch_node->original_network->update();

				branch_node->original_network->epoch_iter = 0;
			}
		}
	}

	for (set<BranchNode*>::iterator it = hit_branch.begin();
			it != hit_branch.end(); it++) {
		BranchNode* branch_node = *it;

		branch_node->branch_network->epoch_iter++;
		if (branch_node->is_ramp) {
			if (branch_node->branch_network->epoch_iter >= RAMP_EPOCH_SIZE) {
				branch_node->branch_network->update();

				branch_node->branch_network->epoch_iter = 0;
			}
		} else {
			if (branch_node->branch_network->epoch_iter >= UPDATE_EPOCH_SIZE) {
				branch_node->branch_network->update();

				branch_node->branch_network->epoch_iter = 0;
			}
		}
	}

	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		Scope* scope = wrapper->solution->scopes[s_index];
		for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
				it != scope->nodes.end(); it++) {
			switch (it->second->type) {
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
