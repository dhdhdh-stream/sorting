#include "new_action_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewActionExperiment::add_new_test_location(ScopeHistory* scope_history) {
	uniform_int_distribution<int> distribution(0, scope_history->node_histories.size()-1);
	int index_1 = distribution(generator);
	int index_2 = distribution(generator);
	if (index_1 != index_2) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it_1 = next(scope_history->node_histories.begin(), index_1);
		map<AbstractNode*, AbstractNodeHistory*>::iterator it_2 = next(scope_history->node_histories.begin(), index_2);

		AbstractNode* potential_start;
		bool potential_is_branch;
		AbstractNode* potential_exit;
		if (it_1->second->index > it_2->second->index) {
			potential_start = it_2->first;
			switch (it_2->first->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_SCOPE:
				potential_is_branch = false;
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it_2->second;
					potential_is_branch = branch_node_history->is_branch;
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it_2->second;
					potential_is_branch = info_branch_node_history->is_branch;
				}
				break;
			}
			potential_exit = it_1->first;
		} else {
			potential_start = it_1->first;
			switch (it_1->first->type) {
			case NODE_TYPE_ACTION:
			case NODE_TYPE_SCOPE:
				potential_is_branch = false;
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it_1->second;
					potential_is_branch = branch_node_history->is_branch;
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it_1->second;
					potential_is_branch = info_branch_node_history->is_branch;
				}
				break;
			}
			potential_exit = it_2->first;
		}

		bool has_match = false;
		for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
			if (this->test_location_starts[t_index] == potential_start
					&& this->test_location_is_branch[t_index] == potential_is_branch) {
				has_match = true;
				break;
			}
		}
		if (!has_match) {
			for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
				if (this->successful_location_starts[s_index] == potential_start
						&& this->successful_location_is_branch[s_index] == potential_is_branch) {
					has_match = true;
					break;
				}
			}
		}

		if (!has_match) {
			this->test_location_starts.push_back(potential_start);
			this->test_location_is_branch.push_back(potential_is_branch);
			this->test_location_exits.push_back(potential_exit);
			this->test_location_states.push_back(NEW_ACTION_EXPERIMENT_MEASURE_EXISTING);
			this->test_location_state_iters.push_back(0);
			this->test_location_existing_scores.push_back(0.0);
			this->test_location_existing_counts.push_back(0);
			this->test_location_new_scores.push_back(0.0);
			this->test_location_new_counts.push_back(0);

			this->average_remaining_experiments_from_start = 1.0;

			potential_start->experiments.insert(potential_start->experiments.begin(), this);
		} else {
			this->generalize_iter++;
			if (this->generalize_iter >= NEW_ACTION_NUM_GENERALIZE_TRIES) {
				if (this->successful_location_starts.size() >= NEW_ACTION_MIN_LOCATIONS) {
					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			}
		}
	}
}
