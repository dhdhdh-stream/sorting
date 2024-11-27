#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::add_new_test_location(NewScopeExperimentHistory* history) {
	int max_index;
	if (this->new_scope_experiment_type == NEW_SCOPE_EXPERIMENT_TYPE_IN_PLACE) {
		max_index = (int)(0.8*((int)history->selected_path_nodes.size()-1));
	} else {
		max_index = (int)history->selected_path_nodes.size();
	}

	vector<AbstractNode*> possible_starts;
	vector<int> possible_start_indexes;
	vector<bool> possible_is_branch;
	for (int h_index = 0; h_index < max_index; h_index++) {
		bool has_match = false;
		for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
			if (this->test_location_starts[t_index] == history->selected_path_nodes[h_index]
					&& this->test_location_is_branch[t_index] == history->selected_path_is_branch[h_index]) {
				has_match = true;
				break;
			}
		}
		if (!has_match) {
			for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
				if (this->successful_location_starts[s_index] == history->selected_path_nodes[h_index]
						&& this->successful_location_is_branch[s_index] == history->selected_path_is_branch[h_index]) {
					has_match = true;
					break;
				}
			}
		}
		if (!has_match) {
			possible_starts.push_back(history->selected_path_nodes[h_index]);
			possible_start_indexes.push_back(h_index);
			possible_is_branch.push_back(history->selected_path_is_branch[h_index]);
		}
	}

	if (possible_starts.size() == 0) {
		this->generalize_iter++;
	} else {
		uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
		int start_index = start_distribution(generator);

		AbstractNode* exit_node;
		if (this->new_scope_experiment_type == NEW_SCOPE_EXPERIMENT_TYPE_IN_PLACE) {
			exit_node = history->selected_path_nodes[possible_start_indexes[start_index] + 1];
		} else {
			if (possible_start_indexes[start_index] == (int)history->selected_path_nodes.size()-1) {
				exit_node = NULL;
			} else {
				uniform_int_distribution<int> exit_distribution(
					possible_start_indexes[start_index] + 1, (int)history->selected_path_nodes.size()-1);
				exit_node = history->selected_path_nodes[exit_distribution(generator)];
			}
		}

		this->test_location_starts.push_back(possible_starts[start_index]);
		this->test_location_is_branch.push_back(possible_is_branch[start_index]);
		this->test_location_exits.push_back(exit_node);
		this->test_location_states.push_back(NEW_SCOPE_EXPERIMENT_MEASURE);
		this->test_location_scores.push_back(0.0);
		this->test_location_counts.push_back(0);
		this->test_location_truth_counts.push_back(0);
		this->test_scope_nodes.push_back(new ScopeNode());

		this->average_remaining_experiments_from_start = 1.0;

		possible_starts[start_index]->experiments.insert(possible_starts[start_index]->experiments.begin(), this);
	}
}
