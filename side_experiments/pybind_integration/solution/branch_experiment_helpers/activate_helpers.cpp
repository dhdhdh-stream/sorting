#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void BranchExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		BranchExperimentHistory* history;
		if (wrapper->experiment_history != NULL) {
			history = (BranchExperimentHistory*)wrapper->experiment_history;
		} else {
			history = new BranchExperimentHistory(this);
			wrapper->experiment_history = history;
		}

		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_check_activate(wrapper,
										  history);
			break;
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			explore_check_activate(wrapper,
								   history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_check_activate(wrapper,
									 history);
			break;
		case BRANCH_EXPERIMENT_STATE_MEASURE:
			measure_check_activate(wrapper);
			break;
		}
	}
}

void BranchExperiment::experiment_step(vector<double>& obs,
									   string& action,
									   bool& is_next,
									   bool& fetch_action,
									   SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper,
					 experiment_state);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper,
					   experiment_state);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper,
					 experiment_state);
		break;
	}
}

void BranchExperiment::set_action(string action,
								  SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();
	explore_set_action(action,
					   experiment_state);
}

void BranchExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper,
						  experiment_state);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper,
							experiment_state);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_exit_step(wrapper,
						  experiment_state);
		break;
	}
}

void BranchExperiment::backprop(double target_val,
								SolutionWrapper* wrapper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								history);
		break;
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 history);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   history);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val);
		break;
	}
}
