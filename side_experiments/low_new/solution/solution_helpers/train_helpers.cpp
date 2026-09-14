/**
 * - OK to train on only paths taken:
 *   - initially, paths not taken are ones that have low score
 *   - since bad paths are no longer taken, score increase overall...
 *   - ...increasing score for bad paths, leading to them being taken again
 *   - ultimately, bad paths will be predicted as bad, but not accurately
 */

#include "solution_helpers.h"

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

const int UPDATE_EPOCH_SIZE = 100;

void train_helper(ScopeHistory* scope_history,
				  double target_val,
				  set<BranchNode*>& hit_original,
				  set<BranchNode*>& hit_branch) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = scope_history->node_histories.begin();
			h_it != scope_history->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)h_it->second;
				train_helper(scope_node_history->scope_history,
							 target_val,
							 hit_original,
							 hit_branch);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;

				if (branch_node_history->is_branch) {
					branch_node->branch_network->activate(branch_node_history->obs);
					double error = target_val - branch_node->branch_network->output->acti_vals(0);
					branch_node->branch_network->backprop(error);

					hit_branch.insert(branch_node);
				} else {
					branch_node->original_network->activate(branch_node_history->obs);
					double error = target_val - branch_node->original_network->output->acti_vals(0);
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
void train_helper(set<BranchNode*>& hit_original,
				  set<BranchNode*>& hit_branch) {
	for (set<BranchNode*>::iterator it = hit_original.begin();
			it != hit_original.end(); it++) {
		BranchNode* branch_node = *it;

		branch_node->original_network->epoch_iter++;
		if (branch_node->original_network->epoch_iter >= UPDATE_EPOCH_SIZE) {
			branch_node->original_network->update();

			branch_node->original_network->epoch_iter = 0;
		}
	}

	for (set<BranchNode*>::iterator it = hit_branch.begin();
			it != hit_branch.end(); it++) {
		BranchNode* branch_node = *it;

		branch_node->branch_network->epoch_iter++;
		if (branch_node->branch_network->epoch_iter >= UPDATE_EPOCH_SIZE) {
			branch_node->branch_network->update();

			branch_node->branch_network->epoch_iter = 0;
		}
	}
}

void train_helper(SolutionWrapper* wrapper) {
	uniform_int_distribution<int> sample_distribution(0, wrapper->train_scope_histories.size()-1);
	for (int iter_index = 0; iter_index < ITERS_PER_BATCH; iter_index++) {
		int index = sample_distribution(generator);

		set<BranchNode*> hit_original;
		set<BranchNode*> hit_branch;
		train_helper(wrapper->train_scope_histories[index],
					 wrapper->train_target_val_histories[index],
					 hit_original,
					 hit_branch);
		train_helper(hit_original,
					 hit_branch);
	}

	for (int h_index = 0; h_index < (int)wrapper->train_scope_histories.size(); h_index++) {
		delete wrapper->train_scope_histories[h_index];
	}
	wrapper->train_scope_histories.clear();
	wrapper->train_target_val_histories.clear();
}
