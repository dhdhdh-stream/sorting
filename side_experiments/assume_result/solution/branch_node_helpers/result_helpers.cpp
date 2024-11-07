#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void BranchNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	Minesweeper* minesweeper = (Minesweeper*)problem;

	run_helper.num_analyze++;

	vector<double> input_vals;
	input_vals.reserve((1 + 2*this->analyze_size) * (1 + 2*this->analyze_size) + 2);
	for (int x_index = -this->analyze_size; x_index < this->analyze_size+1; x_index++) {
		for (int y_index = -this->analyze_size; y_index < this->analyze_size+1; y_index++) {
			input_vals.push_back(minesweeper->get_observation_helper(
					minesweeper->current_x + x_index,
					minesweeper->current_y + y_index));
		}
	}
	// input_vals.push_back(minesweeper->current_x);
	// input_vals.push_back(minesweeper->current_y);
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
	if (run_helper.experiments_seen_order.size() == 0) {
		if (solution->subproblem_id == -1
				|| this->parent->id >= solution->subproblem_id) {
			map<pair<AbstractNode*,bool>, int>::iterator it = run_helper.nodes_seen.find({this, is_branch});
			if (it == run_helper.nodes_seen.end()) {
				run_helper.nodes_seen[{this, is_branch}] = 1;
			} else {
				it->second++;
			}
		}
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->result_activate(
			this,
			is_branch,
			curr_node,
			problem,
			context,
			run_helper);
		if (is_selected) {
			return;
		}
	}
}
