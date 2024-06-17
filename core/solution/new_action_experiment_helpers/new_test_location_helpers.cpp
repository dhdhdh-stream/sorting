#include "new_action_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewActionExperiment::add_new_test_location(ScopeHistory* scope_history) {
	vector<AbstractNode*> node_sequence(scope_history->node_histories.size());

	vector<AbstractNode*> possible_starts;
	vector<int> possible_start_indexes;
	vector<bool> possible_is_branch;
	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		node_sequence[it->second->index] = it->first;

		switch (it->first->type) {
		case NODE_TYPE_ACTION:
		case NODE_TYPE_SCOPE:
			{
				bool has_match = false;
				for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
					if (this->test_location_starts[t_index] == it->first
							&& this->test_location_is_branch[t_index] == false) {
						has_match = true;
						break;
					}
				}
				if (!has_match) {
					for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
						if (this->successful_location_starts[s_index] == it->first
								&& this->successful_location_is_branch[s_index] == false) {
							has_match = true;
							break;
						}
					}
				}
				if (!has_match) {
					possible_starts.push_back(it->first);
					possible_start_indexes.push_back(it->second->index);
					possible_is_branch.push_back(false);
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;

				bool is_branch = branch_node_history->score >= 0.0;

				bool has_match = false;
				for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
					if (this->test_location_starts[t_index] == it->first
							&& this->test_location_is_branch[t_index] == is_branch) {
						has_match = true;
						break;
					}
				}
				if (!has_match) {
					for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
						if (this->successful_location_starts[s_index] == it->first
								&& this->successful_location_is_branch[s_index] == is_branch) {
							has_match = true;
							break;
						}
					}
				}
				if (!has_match) {
					possible_starts.push_back(it->first);
					possible_start_indexes.push_back(it->second->index);
					possible_is_branch.push_back(is_branch);
				}
			}
			break;
		case NODE_TYPE_INFO_BRANCH:
			{
				InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
				InfoBranchNode* info_branch_node = (InfoBranchNode*)it->first;

				bool is_branch;
				if (info_branch_node->is_negate) {
					is_branch = info_branch_node_history->score < 0.0;
				} else {
					is_branch = info_branch_node_history->score >= 0.0;
				}

				bool has_match = false;
				for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
					if (this->test_location_starts[t_index] == it->first
							&& this->test_location_is_branch[t_index] == is_branch) {
						has_match = true;
						break;
					}
				}
				if (!has_match) {
					for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
						if (this->successful_location_starts[s_index] == it->first
								&& this->successful_location_is_branch[s_index] == is_branch) {
							has_match = true;
							break;
						}
					}
				}
				if (!has_match) {
					possible_starts.push_back(it->first);
					possible_start_indexes.push_back(it->second->index);
					possible_is_branch.push_back(is_branch);
				}
			}
			break;
		}
	}

	if (possible_starts.size() == 0) {
		this->generalize_iter++;
	} else {
		uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
		int start_index = start_distribution(generator);

		AbstractNode* exit_node;
		if (possible_start_indexes[start_index] == (int)node_sequence.size()-1) {
			exit_node = NULL;
		} else {
			uniform_int_distribution<int> exit_distribution(
				possible_start_indexes[start_index] + 1, (int)node_sequence.size()-1);
			exit_node = node_sequence[exit_distribution(generator)];
		}

		this->test_location_starts.push_back(possible_starts[start_index]);
		this->test_location_is_branch.push_back(possible_is_branch[start_index]);
		this->test_location_exits.push_back(exit_node);
		this->test_location_states.push_back(NEW_ACTION_EXPERIMENT_MEASURE_EXISTING);
		this->test_location_existing_scores.push_back(0.0);
		this->test_location_existing_counts.push_back(0);
		this->test_location_existing_truth_counts.push_back(0);
		this->test_location_new_scores.push_back(0.0);
		this->test_location_new_counts.push_back(0);
		this->test_location_new_truth_counts.push_back(0);

		this->average_remaining_experiments_from_start = 1.0;

		possible_starts[start_index]->experiments.insert(possible_starts[start_index]->experiments.begin(), this);
	}
}
