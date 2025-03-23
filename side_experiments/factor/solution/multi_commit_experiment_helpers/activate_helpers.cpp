#include "multi_commit_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void MultiCommitExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	if (this->result != EXPERIMENT_RESULT_SUCCESS || this->state == MULTI_COMMIT_EXPERIMENT_STATE_OUTER_MEASURE) {
		MultiCommitExperimentHistory* history;
		if (this->is_branch == is_branch) {
			map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
				= run_helper.multi_experiment_histories.find(this);
			if (it == run_helper.multi_experiment_histories.end()) {
				history = new MultiCommitExperimentHistory(this);
				run_helper.multi_experiment_histories[this] = history;
			} else {
				history = (MultiCommitExperimentHistory*)it->second;
			}

			run_helper.num_multi_instances++;

			switch (this->state) {
			case MULTI_COMMIT_EXPERIMENT_STATE_EXISTING_GATHER:
				existing_gather_activate(scope_history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
				train_existing_activate(run_helper,
										scope_history,
										history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_EXPLORE:
				explore_activate(curr_node,
								 problem,
								 run_helper,
								 scope_history,
								 history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_FIND_SAVE:
				find_save_activate(curr_node,
								   problem,
								   run_helper,
								   history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER:
				commit_existing_gather_activate(curr_node,
												problem,
												run_helper,
												scope_history,
												history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
				commit_train_existing_activate(curr_node,
											   problem,
											   run_helper,
											   scope_history,
											   history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_NEW_GATHER:
				commit_new_gather_activate(curr_node,
										   problem,
										   run_helper,
										   scope_history,
										   history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
				commit_train_new_activate(curr_node,
										  problem,
										  run_helper,
										  scope_history,
										  history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_MEASURE:
				measure_activate(curr_node,
								 problem,
								 run_helper,
								 scope_history,
								 history);
				break;
			case MULTI_COMMIT_EXPERIMENT_STATE_OUTER_MEASURE:
				outer_measure_activate(curr_node,
									   problem,
									   run_helper,
									   scope_history,
									   history);
				break;
			}
		}
	}
}

void MultiCommitExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	MultiCommitExperimentHistory* history = (MultiCommitExperimentHistory*)run_helper.multi_experiment_histories[this];
	switch (this->state) {
	case MULTI_COMMIT_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_backprop();
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper,
								history);
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper,
						 history);
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_FIND_SAVE:
		find_save_backprop(target_val,
						   run_helper,
						   history);
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER:
		commit_existing_gather_backprop(history);
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_EXISTING:
		commit_train_existing_backprop(target_val,
									   run_helper,
									   history);
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_NEW_GATHER:
		commit_new_gather_backprop(history);
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_TRAIN_NEW:
		commit_train_new_backprop(target_val,
								  run_helper,
								  history);
		break;
	case MULTI_COMMIT_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper,
						 history);
		break;
	}
}
