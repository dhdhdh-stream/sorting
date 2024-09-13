#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"
#include "world_model.h"

using namespace std;

void BranchNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	run_helper.num_analyze += (int)this->input_types.size();

	vector<int> local_location = problem->get_location();

	vector<double> input_vals(this->input_types.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		switch (this->input_types[i_index]) {
		case INPUT_TYPE_GLOBAL:
			if (run_helper.world_model != NULL) {
				vector<int> location = problem_type->relative_to_world(
					context.back().starting_location,
					this->input_locations[i_index]);

				bool is_init;
				double val;
				run_helper.world_model->get_val(location,
												is_init,
												val);

				if (is_init) {
					input_vals[i_index] = val;
				}
			}
			break;
		case INPUT_TYPE_LOCAL:
			if (run_helper.world_model != NULL) {
				vector<int> location = problem_type->relative_to_world(
					local_location,
					this->input_locations[i_index]);

				bool is_init;
				double val;
				run_helper.world_model->get_val(location,
												is_init,
												val);

				if (is_init) {
					input_vals[i_index] = val;
				}
			}
			break;
		case INPUT_TYPE_HISTORY:
			{
				map<AbstractNode*, pair<vector<int>,vector<double>>>::iterator it
					= context.back().node_history.find(this->input_node_contexts[i_index]);
				if (it != context.back().node_history.end()) {
					input_vals[i_index] = it->second.second[this->input_obs_indexes[i_index]];
				}
			}
			break;
		}
	}
	this->network->activate(input_vals);

	bool is_branch;
	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (this->network->output->acti_vals[0] >= 0.0) {
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

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
}
