#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

const double ACTIONS_PER_TEST = 40.0;

void NewScopeExperiment::activate(AbstractNode* experiment_node,
								  bool is_branch,
								  AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	bool has_match = false;
	bool is_test;
	int location_index;
	for (int t_index = 0; t_index < (int)this->test_starts.size(); t_index++) {
		if (this->test_starts[t_index] == experiment_node
				&& this->test_is_branch[t_index] == is_branch) {
			has_match = true;
			is_test = true;
			location_index = t_index;
			break;
		}
	}
	if (!has_match) {
		for (int s_index = 0; s_index < (int)this->successful_starts.size(); s_index++) {
			if (this->successful_starts[s_index] == experiment_node
					&& this->successful_is_branch[s_index] == is_branch) {
				has_match = true;
				is_test = false;
				location_index = s_index;
				break;
			}
		}
	}

	if (has_match) {
		NewScopeExperimentOverallHistory* overall_history;
		map<AbstractExperiment*, AbstractExperimentOverallHistory*>::iterator it
			= run_helper.overall_histories.find(this);
		if (it == run_helper.overall_histories.end()) {
			overall_history = new NewScopeExperimentOverallHistory(this);
			run_helper.overall_histories[this] = overall_history;
		} else {
			overall_history = (NewScopeExperimentOverallHistory*)it->second;
		}

		if (is_test) {
			test_activate(location_index,
						  curr_node,
						  problem,
						  run_helper,
						  overall_history);
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
			this->new_scope->activate(problem,
				run_helper,
				inner_scope_history);
			delete inner_scope_history;

			curr_node = this->successful_exits[location_index];
		}
	}
}

void NewScopeExperiment::back_activate(RunHelper& run_helper,
									   ScopeHistory* scope_history) {
	if (this->generalize_iter != -1) {
		int num_test_locations_seen = 0;
		for (int t_index = 0; t_index < (int)this->test_starts.size(); t_index++) {
			map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
				this->test_starts[t_index]->id);
			if (it != scope_history->node_histories.end()) {
				if (this->test_starts[t_index]->type == NODE_TYPE_BRANCH) {
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch == this->test_is_branch[t_index]) {
						num_test_locations_seen++;
					}
				} else {
					num_test_locations_seen++;
				}
			}
		}

		double expected_number_of_experiments = (int)scope_history->node_histories.size() / ACTIONS_PER_TEST;
		if (expected_number_of_experiments > num_test_locations_seen) {
			vector<AbstractNode*> possible_starts;
			vector<bool> possible_is_branch;
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				if (it->second->node->experiment == NULL) {
					bool is_branch;
					switch (it->second->node->type) {
					case NODE_TYPE_ACTION:
					case NODE_TYPE_SCOPE:
					case NODE_TYPE_OBS:
						is_branch = false;
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
							is_branch = branch_node_history->is_branch;
						}
						break;
					}

					bool has_match = false;
					for (int t_index = 0; t_index < (int)this->test_starts.size(); t_index++) {
						if (this->test_starts[t_index] == it->second->node
								&& this->test_is_branch[t_index] == is_branch) {
							has_match = true;
							break;
						}
					}
					if (!has_match) {
						for (int s_index = 0; s_index < (int)this->successful_starts.size(); s_index++) {
							if (this->successful_starts[s_index] == it->second->node
									&& this->successful_is_branch[s_index] == is_branch) {
								has_match = true;
								break;
							}
						}
					}
					if (!has_match) {
						possible_starts.push_back(it->second->node);
						possible_is_branch.push_back(is_branch);
					}
				}
			}

			if (possible_starts.size() > 0) {
				uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
				int start_index = start_distribution(generator);

				AbstractNode* new_start_node = possible_starts[start_index];
				bool new_is_branch = possible_is_branch[start_index];

				vector<AbstractNode*> possible_exits;

				AbstractNode* starting_node;
				switch (new_start_node->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* action_node = (ActionNode*)new_start_node;
						starting_node = action_node->next_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* scope_node = (ScopeNode*)new_start_node;
						starting_node = scope_node->next_node;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* branch_node = (BranchNode*)new_start_node;
						if (new_is_branch) {
							starting_node = branch_node->branch_next_node;
						} else {
							starting_node = branch_node->original_next_node;
						}
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* obs_node = (ObsNode*)new_start_node;
						starting_node = obs_node->next_node;
					}
					break;
				}

				this->scope_context->random_exit_activate(
					starting_node,
					possible_exits);

				uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
				int random_index = distribution(generator);
				AbstractNode* new_exit_next_node = possible_exits[random_index];

				this->test_starts.push_back(new_start_node);
				this->test_is_branch.push_back(new_is_branch);
				this->test_exits.push_back(new_exit_next_node);
				this->test_existing_scores.push_back(0.0);
				this->test_existing_counts.push_back(0);
				this->test_new_scores.push_back(0.0);
				this->test_new_counts.push_back(0);

				new_start_node->experiment = this;

				NewScopeExperimentOverallHistory* overall_history;
				map<AbstractExperiment*, AbstractExperimentOverallHistory*>::iterator it
					= run_helper.overall_histories.find(this);
				if (it == run_helper.overall_histories.end()) {
					overall_history = new NewScopeExperimentOverallHistory(this);
					run_helper.overall_histories[this] = overall_history;
				} else {
					overall_history = (NewScopeExperimentOverallHistory*)it->second;
				}

				overall_history->test_is_new[this->test_starts.size()-1] = false;
			}
		}
	}
}

void NewScopeExperiment::backprop(AbstractExperimentInstanceHistory* instance_history,
								  double target_val) {
	// do nothing
}

void NewScopeExperiment::update(AbstractExperimentOverallHistory* overall_history,
								double target_val) {
	NewScopeExperimentOverallHistory* new_scope_experiment_overall_history = (NewScopeExperimentOverallHistory*)overall_history;

	test_update(new_scope_experiment_overall_history,
				target_val);
}
