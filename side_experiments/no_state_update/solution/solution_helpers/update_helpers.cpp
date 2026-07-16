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

#if defined(MDEBUG) && MDEBUG
const int ITERS_PER_RAMP = 2;
#else
const int ITERS_PER_RAMP = 4000;
#endif /* MDEBUG */

void update_helper(ScopeHistory* scope_history,
				   set<BranchNode*>& hit_original,
				   set<BranchNode*>& hit_branch) {
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

				update_helper(scope_node_history->scope_history,
							  hit_original,
							  hit_branch);

				scope_node->curr_num_instances++;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					branch_node->branch_curr_num_instances++;

					hit_branch.insert(branch_node);
				} else {
					branch_node->original_curr_num_instances++;

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
void update_helper(double target_val,
				   set<BranchNode*>& hit_original,
				   set<BranchNode*>& hit_branch,
				   SolutionWrapper* wrapper) {
	if (wrapper->run_type != RUN_TYPE_EXPLORE) {
		for (int h_index = (int)wrapper->network_histories.size()-1; h_index >= 0; h_index--) {
			switch (wrapper->network_histories[h_index]->network->type) {
			case NETWORK_TYPE_SCORE:
				{
					ScoreNetworkHistory* score_network_history = (ScoreNetworkHistory*)wrapper->network_histories[h_index];
					ScoreNetwork* score_network = (ScoreNetwork*)score_network_history->network;
					score_network->load(score_network_history);
					score_network->backprop(target_val);
				}
				break;
			}
		}
		for (int h_index = (int)wrapper->network_histories.size()-1; h_index >= 0; h_index--) {
			delete wrapper->network_histories[h_index];
		}
		wrapper->network_histories.clear();
	}

	for (set<BranchNode*>::iterator it = hit_original.begin();
			it != hit_original.end(); it++) {
		BranchNode* branch_node = *it;

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
	}

	for (set<BranchNode*>::iterator it = hit_branch.begin();
			it != hit_branch.end(); it++) {
		BranchNode* branch_node = *it;

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
