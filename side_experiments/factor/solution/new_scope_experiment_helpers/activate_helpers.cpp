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

void NewScopeExperiment::pre_activate(RunHelper& run_helper) {
	if (run_helper.experiment_history == NULL) {
		run_helper.experiment_history = new NewScopeExperimentHistory(this);
	}
}

void NewScopeExperiment::activate(ObsNode* experiment_node,
								  AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	if (run_helper.experiment_history != NULL
			&& run_helper.experiment_history->experiment == this) {
		bool has_match = false;
		bool is_test;
		int location_index;
		if (this->test_location_start == experiment_node) {
			has_match = true;
			is_test = true;
		}
		if (!has_match) {
			for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
				if (this->successful_location_starts[s_index] == experiment_node) {
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
					test_activate(curr_node,
								  problem,
								  run_helper,
								  history);
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
					this->new_scope->experiment_activate(problem,
														 run_helper,
														 inner_scope_history);
					delete inner_scope_history;

					curr_node = this->successful_obs_nodes[location_index];
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
		if (this->test_location_start == NULL) {
			uniform_int_distribution<int> select_distribution(0, history->instance_count);
			if (select_distribution(generator) == 0) {
				vector<ObsNode*> possible_starts;
				vector<bool> possible_is_branch;
				for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
						it != scope_history->node_histories.end(); it++) {
					if (it->second->node->type == NODE_TYPE_OBS) {
						ObsNode* obs_node = (ObsNode*)it->second->node;
						if (obs_node->experiment == NULL
								&& obs_node->average_instances_per_run >= NEW_SCOPE_EXPERIMENT_MIN_INSTANCES_PER_RUN) {
							bool has_match = false;
							for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
								if (this->successful_location_starts[s_index] == obs_node) {
									has_match = true;
									break;
								}
							}
							if (!has_match) {
								possible_starts.push_back(obs_node);
							}
						}
					}
				}

				if (possible_starts.size() > 0) {
					uniform_int_distribution<int> start_distribution(0, possible_starts.size()-1);
					int start_index = start_distribution(generator);

					history->potential_start = possible_starts[start_index];
				}
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
		if (history->hit_test) {
			test_backprop(target_val,
						  run_helper,
						  history);
		} else if (this->test_location_start == NULL) {
			if (history->potential_start != NULL) {
				vector<AbstractNode*> possible_exits;

				AbstractNode* starting_node = history->potential_start->next_node;
				this->scope_context->random_exit_activate(
					starting_node,
					possible_exits);

				uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
				int random_index = distribution(generator);
				AbstractNode* exit_next_node = possible_exits[random_index];

				this->test_location_start = history->potential_start;
				this->test_location_exit = exit_next_node;
				this->test_location_state = LOCATION_STATE_MEASURE_EXISTING;
				this->test_location_existing_score = 0.0;
				this->test_location_new_score = 0.0;
				this->test_location_count = 0;

				history->potential_start->experiment = this;
			}
		}
		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
