#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "solution.h"
#include "solution_set.h"
#include "utilities.h"

using namespace std;

void BranchNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	bool is_branch;
	if (run_helper.branch_node_ancestors.find(this) != run_helper.branch_node_ancestors.end()) {
		is_branch = false;
	} else {
		run_helper.num_analyze += (1 + 2*this->analyze_size) * (1 + 2*this->analyze_size);

		vector<vector<double>> input_vals(1 + 2*this->analyze_size);
		for (int x_index = 0; x_index < 1 + 2*this->analyze_size; x_index++) {
			input_vals[x_index] = vector<double>(1 + 2*this->analyze_size);
		}

		Minesweeper* minesweeper = (Minesweeper*)problem;
		for (int x_index = -this->analyze_size; x_index < this->analyze_size+1; x_index++) {
			for (int y_index = -this->analyze_size; y_index < this->analyze_size+1; y_index++) {
				input_vals[x_index + this->analyze_size][y_index + this->analyze_size]
					= minesweeper->get_observation_helper(
						minesweeper->current_x + x_index,
						minesweeper->current_y + y_index);
			}
		}
		this->network->activate(input_vals);

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

		run_helper.num_actions++;
		Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
		if (run_helper.num_actions > solution->num_actions_limit) {
			run_helper.exceeded_limit = true;
			return;
		}
		context.back().nodes_seen.push_back({this, is_branch});
		context.back().location_history[this] = {minesweeper->current_x, minesweeper->current_y};
	}

	if (is_branch) {
		run_helper.branch_node_ancestors.insert(this);

		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		bool is_selected = this->experiments[e_index]->activate(
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
