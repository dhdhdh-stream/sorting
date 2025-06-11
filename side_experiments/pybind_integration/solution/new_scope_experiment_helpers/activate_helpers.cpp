#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

void NewScopeExperiment::pre_activate(SolutionWrapper* wrapper) {
	if (wrapper->experiment_history == NULL) {
		wrapper->experiment_history = new NewScopeExperimentHistory(this);
	}
}

void NewScopeExperiment::check_activate(AbstractNode* experiment_node,
										bool is_branch,
										SolutionWrapper* wrapper) {
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
		NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)wrapper->experiment_history;

		if (is_test) {
			history->hit_test = true;

			switch (this->test_location_state) {
			case LOCATION_STATE_MEASURE_NEW:
			case LOCATION_STATE_VERIFY_NEW_1ST:
			case LOCATION_STATE_VERIFY_NEW_2ND:
				{
					NewScopeExperimentState* new_experiment_state = new NewScopeExperimentState(this);
					wrapper->experiment_context.back() = new_experiment_state;

					ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
					wrapper->scope_histories.push_back(inner_scope_history);
					wrapper->node_context.push_back(this->new_scope->nodes[0]);
					wrapper->experiment_context.push_back(NULL);
					wrapper->confusion_context.push_back(NULL);
				}
				break;
			}
		} else {
			wrapper->node_context.back() = this->successful_scope_nodes[location_index];
		}
	}
}

void NewScopeExperiment::experiment_step(vector<double>& obs,
										 string& action,
										 bool& is_next,
										 bool& fetch_action,
										 SolutionWrapper* wrapper) {
	// can't be hit
}

void NewScopeExperiment::set_action(string action,
									SolutionWrapper* wrapper) {
	// can't be hit
}

void NewScopeExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();
	wrapper->confusion_context.pop_back();

	wrapper->node_context.back() = this->test_location_exit;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void NewScopeExperiment::back_activate(SolutionWrapper* wrapper) {
	if (this->test_location_start == NULL) {
		NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)wrapper->experiment_history;
		ScopeHistory* scope_history = wrapper->scope_histories.back();

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

void NewScopeExperiment::backprop(double target_val,
								  SolutionWrapper* wrapper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)wrapper->experiment_history;

	if (history->hit_test) {
		test_backprop(target_val,
					  history);
	} else if (this->test_location_start == NULL) {
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
			this->test_location_state = LOCATION_STATE_MEASURE_EXISTING;
			this->test_location_existing_score = 0.0;
			this->test_location_new_score = 0.0;
			this->test_location_count = 0;

			history->potential_start->experiment = this;
		}
	}
}
