#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void BranchExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	if (this->is_branch == is_branch) {
		run_helper.num_experiment_instances++;

		map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
			= run_helper.experiment_histories.find(this);
		BranchExperimentHistory* history;
		if (it == run_helper.experiment_histories.end()) {
			history = new BranchExperimentHistory(this);

			uniform_int_distribution<int> experiment_active_distribution(0, 2);
			switch (this->state) {
			case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
			case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
			case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
				history->is_active = false;
				break;
			case BRANCH_EXPERIMENT_STATE_EXPLORE:
				if (run_helper.has_explore) {
					history->is_active = false;
				} else {
					run_helper.has_explore = true;
					history->is_active = true;
				}
				break;
			case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			case BRANCH_EXPERIMENT_STATE_MEASURE:
				history->is_active = experiment_active_distribution(generator) == 0;
				break;
			}

			run_helper.experiment_histories[this] = history;
		} else {
			history = (BranchExperimentHistory*)it->second;
		}

		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
			existing_gather_activate(scope_history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(run_helper,
									scope_history,
									history);
			break;
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history,
							 history);
			break;
		case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
			new_gather_activate(scope_history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node,
							   problem,
							   run_helper,
							   scope_history,
							   history);
			break;
		case BRANCH_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history,
							 history);
			break;
		}
	}
}

void BranchExperiment::backprop(double target_val,
								bool is_return,
								RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories[this];
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								is_return,
								run_helper,
								history);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 is_return,
						 run_helper,
						 history);
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   is_return,
						   run_helper,
						   history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 is_return,
						 run_helper);
		break;
	}
}
