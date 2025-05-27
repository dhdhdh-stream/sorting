#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void CommitExperiment::result_activate(AbstractNode* experiment_node,
									   bool is_branch,
									   AbstractNode*& curr_node,
									   Problem* problem,
									   RunHelper& run_helper,
									   ScopeHistory* scope_history) {
	bool is_selected = false;
	CommitExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history != NULL) {
			history = (CommitExperimentHistory*)run_helper.experiment_history;
			is_selected = true;
		} else if (run_helper.experiment_history == NULL) {
			history = new CommitExperimentHistory(this);
			run_helper.experiment_history = history;
			is_selected = true;
		}
	}

	if (is_selected) {
		switch (this->state) {
		case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
			for (int n_index = 0; n_index < this->step_iter; n_index++) {
				switch (this->new_nodes[n_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* node = (ActionNode*)this->new_nodes[n_index];
						node->commit_activate(problem,
											  run_helper,
											  scope_history);
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
						node->commit_activate(problem,
											  run_helper,
											  scope_history);
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* node = (ObsNode*)this->new_nodes[n_index];
						node->commit_activate(problem,
											  run_helper,
											  scope_history);
					}
					break;
				}
			}

			run_helper.num_actions++;

			for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
				if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->save_actions[s_index]);
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[s_index]);
					this->save_scopes[s_index]->result_activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}

				run_helper.num_actions += 2;
			}

			curr_node = this->save_exit_next_node;

			break;
		}
	}
}

void CommitExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	bool is_selected = false;
	CommitExperimentHistory* history = NULL;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history != NULL) {
			history = (CommitExperimentHistory*)run_helper.experiment_history;
			is_selected = true;
		} else if (run_helper.experiment_history == NULL) {
			history = new CommitExperimentHistory(this);
			run_helper.experiment_history = history;
			is_selected = true;
		}
	}

	if (is_selected) {
		switch (this->state) {
		case COMMIT_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history,
							 history);
			break;
		case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
			find_save_activate(curr_node,
							   problem,
							   run_helper,
							   scope_history);
			break;
		case COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER:
			commit_existing_gather_activate(curr_node,
											problem,
											run_helper,
											scope_history);
			break;
		case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
			commit_train_existing_activate(curr_node,
										   problem,
										   run_helper,
										   scope_history,
										   history);
			break;
		case COMMIT_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_activate(curr_node,
									problem,
									run_helper,
									scope_history);
			break;
		#endif /* MDEBUG */
		}
	}
}

void CommitExperiment::back_activate(RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	switch (this->state) {
	case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
		if (scope_history->has_local_experiment) {
			this->save_match_histories.push_back(
				scope_history->num_matches - scope_history->experiment_num_matches);
		}
		break;
	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
		if (scope_history->has_local_experiment) {
			this->commit_existing_match_histories.push_back(
				scope_history->num_matches - scope_history->experiment_num_matches);
		}
		break;
	}
}

void CommitExperiment::backprop(double target_val,
								RunHelper& run_helper) {
	CommitExperimentHistory* history = (CommitExperimentHistory*)run_helper.experiment_history;
	switch (this->state) {
	case COMMIT_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper,
						 history);
		break;
	case COMMIT_EXPERIMENT_STATE_FIND_SAVE:
		find_save_backprop(target_val,
						   run_helper);
		break;
	case COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER:
		commit_existing_gather_backprop();
		break;
	case COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
		commit_train_existing_backprop(target_val,
									   run_helper,
									   history);
		break;
	case COMMIT_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
