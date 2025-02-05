#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void NewScopeExperiment::pre_activate(RunHelper& run_helper) {
	if (run_helper.experiment_history == NULL) {
		bool has_seen = false;
		for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
			if (run_helper.experiments_seen_order[e_index] == this) {
				has_seen = true;
				break;
			}
		}
		if (!has_seen) {
			double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
			uniform_real_distribution<double> distribution(0.0, 1.0);
			if (distribution(generator) < selected_probability) {
				run_helper.experiment_history = new NewScopeExperimentHistory(this);
			}

			run_helper.experiments_seen_order.push_back(this);
		}
	}
}

void NewScopeExperiment::activate(AbstractNode* experiment_node,
								  bool is_branch,
								  AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	if (run_helper.experiment_history != NULL
			&& run_helper.experiment_history->experiment == this) {
		bool has_match = false;
		bool is_test;
		int location_index;
		for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
			if (this->test_location_starts[t_index] == experiment_node
					&& this->test_location_is_branch[t_index] == is_branch) {
				has_match = true;
				is_test = true;
				location_index = t_index;
				break;
			}
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
			NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

			switch (this->state) {
			case NEW_SCOPE_EXPERIMENT_STATE_EXPLORE:
				if (is_test) {
					if (history->test_location_index == -1
							|| history->test_location_index == location_index) {
						test_activate(location_index,
									  curr_node,
									  problem,
									  run_helper,
									  history);
					}
				} else {
					this->successful_scope_nodes[location_index]->experiment_activate(
						curr_node,
						problem,
						run_helper,
						scope_history);
				}
				break;
			#if defined(MDEBUG) && MDEBUG
			case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
				capture_verify_activate(location_index,
										curr_node,
										problem,
										run_helper,
										scope_history);
				break;
			#endif /* MDEBUG */
			}
		}
	}
}

void NewScopeExperiment::back_activate(RunHelper& run_helper,
									   ScopeHistory* scope_history) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_EXPLORE:
		if (history->test_location_index == -1) {
			uniform_int_distribution<int> select_distribution(0, history->instance_count);
			if (select_distribution(generator) == 0) {
				vector<AbstractNode*> possible_starts;
				vector<bool> possible_is_branch;
				for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
						it != scope_history->node_histories.end(); it++) {
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
					for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
						if (this->test_location_starts[t_index] == it->second->node
								&& this->test_location_is_branch[t_index] == is_branch) {
							has_match = true;
							break;
						}
					}
					if (!has_match) {
						for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
							if (this->successful_location_starts[s_index] == it->second->node
									&& this->successful_location_is_branch[s_index] == is_branch) {
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

				uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
				int start_index = start_distribution(generator);

				history->potential_start = possible_starts[start_index];
				history->potential_is_branch = possible_is_branch[start_index];
			}
			history->instance_count++;
		}

		break;
	}
}

void NewScopeExperiment::backprop(double target_val,
								  RunHelper& run_helper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_EXPLORE:
		if (history->test_location_index != -1) {
			test_backprop(target_val,
						  run_helper);
		} else {
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

			this->test_location_starts.push_back(history->potential_start);
			this->test_location_is_branch.push_back(history->potential_is_branch);
			this->test_location_exits.push_back(exit_next_node);
			this->test_location_states.push_back(LOCATION_STATE_MEASURE_EXISTING);
			this->test_location_existing_scores.push_back(0.0);
			this->test_location_new_scores.push_back(0.0);
			this->test_location_counts.push_back(0);

			history->potential_start->experiments.insert(history->potential_start->experiments.begin(), this);
		}

		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
