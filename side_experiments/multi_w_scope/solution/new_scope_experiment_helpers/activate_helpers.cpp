#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::activate(AbstractNode* experiment_node,
								  bool is_branch,
								  AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	bool has_match = false;
	bool is_test;
	int location_index;
	if (this->test_location_start == experiment_node
			&& this->test_location_is_branch == is_branch) {
		has_match = true;
		is_test = true;
	}
	if (!has_match) {
		for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
			if (this->successful_location_starts[s_index] == experiment_node
					&& this->successful_location_is_branch[s_index] == is_branch) {
				has_match = true;
				is_test = false;
				location_index = s_index;
				break;
			}
		}
	}

	if (has_match) {
		map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
				= run_helper.experiment_histories.find(this);
		NewScopeExperimentHistory* history;
		if (it == run_helper.experiment_histories.end()) {
			history = new NewScopeExperimentHistory(this);

			if (this->test_location_state == LOCATION_STATE_CHECK_LOCATION) {
				if (run_helper.has_explore) {
					history->is_active = false;
				} else {
					run_helper.has_explore = true;
					history->is_active = true;
				}
			} else {
				uniform_int_distribution<int> experiment_active_distribution(0, 2);
				history->is_active = experiment_active_distribution(generator) == 0;
			}

			run_helper.experiment_histories[this] = history;
		} else {
			history = (NewScopeExperimentHistory*)it->second;
		}

		if (is_test) {
			history->hit_test = true;

			if (history->is_active) {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
				this->new_scope->experiment_activate(problem,
													 run_helper,
													 inner_scope_history);
				delete inner_scope_history;

				curr_node = this->test_location_exit;
			}
		} else {
			if (history->is_active) {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
				this->new_scope->experiment_activate(problem,
													 run_helper,
													 inner_scope_history);
				delete inner_scope_history;

				curr_node = this->successful_location_exits[location_index];
			}
		}
	}
}

void NewScopeExperiment::back_activate(RunHelper& run_helper,
									   ScopeHistory* scope_history) {
	run_helper.num_experiment_instances++;

	if (this->test_location_start == NULL) {
		map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
				= run_helper.experiment_histories.find(this);
		NewScopeExperimentHistory* history;
		if (it == run_helper.experiment_histories.end()) {
			history = new NewScopeExperimentHistory(this);

			if (this->test_location_state == LOCATION_STATE_CHECK_LOCATION) {
				if (run_helper.has_explore) {
					history->is_active = false;
				} else {
					run_helper.has_explore = true;
					history->is_active = true;
				}
			} else {
				uniform_int_distribution<int> experiment_active_distribution(0, 2);
				history->is_active = experiment_active_distribution(generator) == 0;
			}

			run_helper.experiment_histories[this] = history;
		} else {
			history = (NewScopeExperimentHistory*)it->second;
		}

		if (history->is_active) {
			uniform_int_distribution<int> select_distribution(0, history->instance_count);
			if (select_distribution(generator) == 0) {
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
						for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
							if (this->successful_location_starts[s_index] == it->second->node
									&& this->successful_location_is_branch[s_index] == is_branch) {
								has_match = true;
								break;
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

					history->potential_start = possible_starts[start_index];
					history->potential_is_branch = possible_is_branch[start_index];
				}
			}
			history->instance_count++;
		}
	}
}

void NewScopeExperiment::backprop(double target_val,
								  bool is_return,
								  RunHelper& run_helper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_histories[this];

	if (history->hit_test) {
		test_backprop(target_val,
					  is_return,
					  run_helper,
					  history);
	} else if (this->test_location_start == NULL
			&& history->is_active) {
		if (history->potential_start != NULL) {
			vector<AbstractNode*> possible_exits;

			AbstractNode* starting_node;
			switch (history->potential_start->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)history->potential_start;
					starting_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)history->potential_start;
					starting_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)history->potential_start;
					if (history->potential_is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)history->potential_start;
					starting_node = obs_node->next_node;
				}
				break;
			}

			this->scope_context->random_exit_activate(
				starting_node,
				possible_exits);

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			AbstractNode* exit_next_node = possible_exits[random_index];

			this->test_location_start = history->potential_start;
			this->test_location_is_branch = history->potential_is_branch;
			this->test_location_exit = exit_next_node;
			this->test_location_state = LOCATION_STATE_CHECK_LOCATION;

			history->potential_start->experiment = this;

			this->multi_index = multi_counter;
			multi_counter++;
		} else {
			this->generalize_iter++;
			if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
