#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"
#include "solution.h"
#include "start_node.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_ITERS = 2;
#else
const int UPDATE_ITERS = 100;
#endif /* MDEBUG */

void update_solution_helper(ExperimentRun* run,
							double target_val) {
	for (map<int, AbstractNodeHistory*>::iterator h_it = run->node_histories.begin();
			h_it != run->node_histories.end(); h_it++) {
		switch (h_it->second->node->type) {
		case NODE_TYPE_START:
			{
				StartNodeHistory* start_node_history = (StartNodeHistory*)h_it->second;
				StartNode* start_node = (StartNode*)start_node_history->node;
				if (start_node->state_history.size() < STATE_NUM_SAVE) {
					start_node->state_history.push_back(start_node_history->state);
				} else {
					start_node->state_history[start_node->history_index] = start_node_history->state;
				}
				start_node->history_index++;
				if (start_node->history_index >= STATE_NUM_SAVE) {
					start_node->history_index = 0;
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)h_it->second;
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				if (action_node->state_history.size() < STATE_NUM_SAVE) {
					action_node->state_history.push_back(action_node_history->state);
				} else {
					action_node->state_history[action_node->history_index] = action_node_history->state;
				}
				action_node->history_index++;
				if (action_node->history_index >= STATE_NUM_SAVE) {
					action_node->history_index = 0;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)h_it->second;
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				if (branch_node_history->is_branch) {
					if (branch_node->branch_state_history.size() < STATE_NUM_SAVE) {
						branch_node->branch_state_history.push_back(branch_node_history->state);
					} else {
						branch_node->branch_state_history[branch_node->branch_history_index] = branch_node_history->state;
					}
					branch_node->branch_history_index++;
					if (branch_node->branch_history_index >= STATE_NUM_SAVE) {
						branch_node->branch_history_index = 0;
					}

					if (branch_node->branch_state_history.size() >= STATE_NUM_SAVE) {
						uniform_int_distribution<int> distribution(0, branch_node->branch_state_history.size()-1);
						for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
							int index = distribution(generator);
							branch_node->branch_network->activate(branch_node->branch_state_history[index]);
							double error = branch_node->branch_target_val_history[index] - branch_node->branch_network->output->acti_vals[0];
							branch_node->branch_network->backprop(error);
						}
					}
				} else {
					if (branch_node->original_state_history.size() < STATE_NUM_SAVE) {
						branch_node->original_state_history.push_back(branch_node_history->state);
					} else {
						branch_node->original_state_history[branch_node->original_history_index] = branch_node_history->state;
					}
					branch_node->original_history_index++;
					if (branch_node->original_history_index >= STATE_NUM_SAVE) {
						branch_node->original_history_index = 0;
					}

					if (branch_node->original_state_history.size() >= STATE_NUM_SAVE) {
						uniform_int_distribution<int> distribution(0, branch_node->original_state_history.size()-1);
						for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
							int index = distribution(generator);
							branch_node->original_network->activate(branch_node->original_state_history[index]);
							double errors = branch_node->original_target_val_history[index] - branch_node->original_network->output->acti_vals[0];
							branch_node->original_network->backprop(errors);
						}
					}
				}
			}
			break;
		}
	}
}
