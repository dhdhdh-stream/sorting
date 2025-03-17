#include "multi_branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void MultiBranchExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	MultiBranchExperimentHistory* history;
	map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
		= run_helper.multi_experiment_histories.find(this);
	if (it == run_helper.multi_experiment_histories.end()) {
		history = new MultiBranchExperimentHistory(this);
		run_helper.multi_experiment_histories[this] = history;
	} else {
		history = (MultiBranchExperimentHistory*)it->second;
	}

	switch (this->state) {
	case MULTI_BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_activate(scope_history);
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_activate(run_helper,
								scope_history,
								history);
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_activate(curr_node,
						 problem,
						 run_helper,
						 scope_history,
						 history);
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_activate(scope_history);
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_activate(curr_node,
						   problem,
						   run_helper,
						   scope_history,
						   history);
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_activate(curr_node,
						 problem,
						 run_helper,
						 scope_history,
						 history);
		break;
	}
}

void MultiBranchExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	MultiBranchExperimentHistory* history = (MultiBranchExperimentHistory*)run_helper.multi_experiment_histories[this];
	switch (this->state) {
	case MULTI_BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_backprop();
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								run_helper,
								history);
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper,
						 history);
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_backprop();
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper,
						   history);
		break;
	case MULTI_BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper,
						 history);
		break;
	}
}
