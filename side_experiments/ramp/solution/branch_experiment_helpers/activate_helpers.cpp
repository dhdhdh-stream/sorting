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

void BranchExperiment::activate(AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	run_helper.num_experiment_instances++;

	map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
		= run_helper.experiment_histories.find(this);
	BranchExperimentHistory* history;
	if (it == run_helper.experiment_histories.end()) {
		history = new BranchExperimentHistory(this);
		run_helper.experiment_histories[this] = history;
	} else {
		history = (BranchExperimentHistory*)it->second;
	}

	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_activate(scope_history);
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
	case BRANCH_EXPERIMENT_STATE_MEASURE_1_PERCENT:
	case BRANCH_EXPERIMENT_STATE_MEASURE_5_PERCENT:
	case BRANCH_EXPERIMENT_STATE_MEASURE_10_PERCENT:
	case BRANCH_EXPERIMENT_STATE_MEASURE_25_PERCENT:
	case BRANCH_EXPERIMENT_STATE_MEASURE_50_PERCENT:
		measure_activate(curr_node,
						 problem,
						 run_helper,
						 scope_history,
						 history);
		break;
	}
}

void BranchExperiment::backprop(double target_val,
								AbstractExperimentHistory* history,
								set<Scope*>& updated_scopes) {
	BranchExperimentHistory* branch_experiment_history = (BranchExperimentHistory*)history;
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 branch_experiment_history);
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   branch_experiment_history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE_1_PERCENT:
	case BRANCH_EXPERIMENT_STATE_MEASURE_5_PERCENT:
	case BRANCH_EXPERIMENT_STATE_MEASURE_10_PERCENT:
	case BRANCH_EXPERIMENT_STATE_MEASURE_25_PERCENT:
	case BRANCH_EXPERIMENT_STATE_MEASURE_50_PERCENT:
		measure_backprop(target_val,
						 branch_experiment_history,
						 updated_scopes);
		break;
	}
}
