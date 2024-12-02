#include "pass_through_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void PassThroughExperiment::activate(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_history;

	explore_activate(curr_node,
					 problem,
					 context,
					 run_helper,
					 scope_history,
					 history);
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	explore_backprop(target_val,
					 run_helper);
}
