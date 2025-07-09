#include "branch_compare_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "factor.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void BranchCompareExperiment::clean() {
	this->node_context->experiment = NULL;
}

void BranchCompareExperiment::add(SolutionWrapper* wrapper) {
	// do nothing
}
