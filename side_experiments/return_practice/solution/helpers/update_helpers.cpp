#include "solution_helpers.h"

#include "branch_node.h"
#include "experiment_run.h"
#include "network.h"
#include "obs_node.h"
#include "solution.h"
#include "wrapper.h"

using namespace std;

void update_solution_helper(ExperimentRun* run,
							double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)h_it->second->node;
				obs_node->sum_instances++;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					branch_node->branch_sum_instances++;

					branch_node->branch_network->activate(branch_node_history->state);
					vector<double> errors{target_val - branch_node->branch_network->output->acti_vals[0]};
					branch_node->branch_network->backprop(errors);

					branch_node->branch_network_epoch_iter++;
					if (branch_node->branch_network_epoch_iter >= NETWORK_EPOCH_SIZE) {
						branch_node->branch_network->update();

						branch_node->branch_network_epoch_iter = 0;
					}
				} else {
					branch_node->original_sum_instances++;

					branch_node->original_network->activate(branch_node_history->state);
					vector<double> errors{target_val - branch_node->original_network->output->acti_vals[0]};
					branch_node->original_network->backprop(errors);

					branch_node->original_network_epoch_iter++;
					if (branch_node->original_network_epoch_iter >= NETWORK_EPOCH_SIZE) {
						branch_node->original_network->update();

						branch_node->original_network_epoch_iter = 0;
					}
				}
			}
			break;
		}
	}

	Solution* solution = run->wrapper->solution;
	for (map<int, AbstractNode*>::iterator it = solution->nodes.begin();
			it != solution->nodes.end(); it++) {
		switch (it->second->type) {
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)it->second;
				obs_node->average_instances_per_run = 0.999 * obs_node->average_instances_per_run
					+ 0.001 * obs_node->sum_instances;
				obs_node->sum_instances = 0;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)it->second;
				branch_node->original_average_instances_per_run = 0.999 * branch_node->original_average_instances_per_run
					+ 0.001 * branch_node->original_sum_instances;
				branch_node->original_sum_instances = 0;
				branch_node->branch_average_instances_per_run = 0.999 * branch_node->branch_average_instances_per_run
					+ 0.001 * branch_node->branch_sum_instances;
				branch_node->branch_sum_instances = 0;
			}
			break;
		}
	}
}
