#include "commit_experiment.h"

#include <iostream>

#include "globals.h"

using namespace std;

void CommitExperiment::activate(AbstractNode* experiment_node,
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
	if (select_distribution(generator) == 0) {
		CommitExperimentHistory* history = new CommitExperimentHistory(this);
		run_helper.experiment_histories.push_back(history);

		switch (this->state) {
		case COMMIT_EXPERIMENT_STATE_EXISTING_GATHER:
			existing_gather_activate(scope_history);
			break;
		case COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(scope_history);
			break;
		case COMMIT_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history,
							 history);
			break;
		case COMMIT_EXPERIMENT_STATE_EXPERIMENT:
			experiment_activate(curr_node,
								problem,
								run_helper,
								scope_history,
								history);
			break;
		}
	}
}

void CommitExperiment::backprop(AbstractExperimentHistory* history,
								double target_val) {
	CommitExperimentHistory* commit_experiment_history = (CommitExperimentHistory*)history;
	switch (this->state) {
	case COMMIT_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_backprop();
		break;
	case COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val);
		break;
	case COMMIT_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(commit_experiment_history,
						 target_val);
		break;
	case COMMIT_EXPERIMENT_STATE_EXPERIMENT:
		experiment_backprop(commit_experiment_history,
							target_val);
		break;
	}
}

void CommitExperiment::update() {
	switch (this->state) {
	case COMMIT_EXPERIMENT_STATE_EXISTING_GATHER:
		existing_gather_update();
		break;
	case COMMIT_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_update();
		break;
	case COMMIT_EXPERIMENT_STATE_EXPLORE:
		explore_update();
		break;
	case COMMIT_EXPERIMENT_STATE_EXPERIMENT:
		experiment_update();
		break;
	}
}
