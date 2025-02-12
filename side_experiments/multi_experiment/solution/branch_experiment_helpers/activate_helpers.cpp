#include "branch_experiment.h"

#include <iostream>

#include "globals.h"

using namespace std;

void BranchExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	run_helper.experiments_seen.insert(this);

	/**
	 * - each experiment has 1/4 chance of triggering
 	 *   - so two experiments can't be more than 1/4 correlated
	 */
	uniform_int_distribution<int> select_distribution(0, 3);
	// if (select_distribution(generator) == 0) {
	if (true) {
		BranchExperimentHistory* history = new BranchExperimentHistory(this);
		run_helper.experiment_histories.push_back(history);

		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
			existing_gather_activate(scope_history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(scope_history);
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
							 scope_history);
			break;
		}
	}
}

void BranchExperiment::backprop(AbstractExperimentHistory* history,
								double target_val) {
	BranchExperimentHistory* branch_experiment_history = (BranchExperimentHistory*)history;
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(branch_experiment_history,
						 target_val);
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_backprop();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(branch_experiment_history,
						   target_val);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val);
		break;
	}
}

void BranchExperiment::update() {
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_update();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_update();
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_update();
		break;
	case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
		new_gather_update();
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_update();
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_update();
		break;
	}
}
