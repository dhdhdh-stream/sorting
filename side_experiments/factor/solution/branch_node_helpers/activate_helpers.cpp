#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void BranchNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	double sum_vals = 0.0;
	for (int f_index = 0; f_index < (int)this->factor_ids.size(); f_index++) {
		map<pair<pair<vector<Scope*>,vector<int>>, pair<int,int>>>::iterator it
			= context.back().obs_history.find(
				{{vector<Scope*>{this->parent},vector<int>{this->factor_ids[f_index].first}},
					{this->factor_ids[f_index].second,-1}});
		if (it != context.back().obs_history.end()) {
			sum_vals += it->second * this->factor_weights[f_index];
		}
	}

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (sum_vals >= 0.0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	#endif /* MDEBUG */

	if (is_branch) {
		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}
}
