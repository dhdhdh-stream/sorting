/**
 * - even updating 1 iter per run makes big difference
 * 
 * TODO: need signals to update more effectively
 */

#include "solution_helpers.h"

#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

void update_helper(double target_val,
				   SolutionWrapper* wrapper) {
	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (map<BranchNode*, pair<int,pair<bool,vector<double>>>>::iterator it = wrapper->branch_node_samples.begin();
			it != wrapper->branch_node_samples.end(); it++) {
		if (it->second.second.first) {
			if (distribution(generator) > it->first->branch_ratio) {
				it->first->branch_network->activate(it->second.second.second);
				double error = target_val - it->first->branch_network->output->acti_vals[0];
				it->first->branch_network->backprop(error);
			}

			it->first->consec_original = 0;
			it->first->consec_branch++;
		} else {
			if (distribution(generator) < it->first->branch_ratio) {
				it->first->original_network->activate(it->second.second.second);
				double error = target_val - it->first->original_network->output->acti_vals[0];
				it->first->original_network->backprop(error);
			}

			it->first->consec_original++;
			it->first->consec_branch = 0;
		}
	}

	wrapper->branch_node_samples.clear();
}
