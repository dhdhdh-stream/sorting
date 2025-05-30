#include "pass_through_experiment.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void PassThroughExperiment::result_activate(AbstractNode* experiment_node,
											bool is_branch,
											AbstractNode*& curr_node,
											Problem* problem,
											RunHelper& run_helper,
											ScopeHistory* scope_history) {
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history == NULL) {
			run_helper.experiment_history = new PassThroughExperimentHistory(this);
		}
	}
}

void PassThroughExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	bool is_selected = false;
	if (is_branch == this->is_branch) {
		if (run_helper.experiment_history != NULL) {
			is_selected = true;
		} else if (run_helper.experiment_history == NULL) {
			run_helper.experiment_history = new PassThroughExperimentHistory(this);
			is_selected = true;
		}
	}

	if (is_selected) {
		explore_activate(curr_node,
						 problem,
						 run_helper,
						 scope_history);
	}
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	explore_backprop(target_val,
					 run_helper);
}
