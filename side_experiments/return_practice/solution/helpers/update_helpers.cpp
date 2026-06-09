#include "solution_helpers.h"

#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "network.h"
#include "solution.h"
#include "wrapper.h"

using namespace std;

void update_solution_helper(ExperimentRun* run,
							double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					branch_node->branch_network->activate(branch_node_history->state);
					vector<double> errors{target_val - branch_node->branch_network->output->acti_vals[0]};
					branch_node->branch_network->backprop(errors);
				} else {
					branch_node->original_network->activate(branch_node_history->state);
					vector<double> errors{target_val - branch_node->original_network->output->acti_vals[0]};
					branch_node->original_network->backprop(errors);
				}
			}
			break;
		}
	}
}
