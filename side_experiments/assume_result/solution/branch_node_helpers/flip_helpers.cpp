#include "branch_node.h"

#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void BranchNode::flip_activate(AbstractNode*& curr_node,
							   Problem* problem,
							   vector<ContextLayer>& context,
							   RunHelper& run_helper) {
	Minesweeper* minesweeper = (Minesweeper*)problem;

	run_helper.num_analyze += (1 + 2*this->analyze_size) * (1 + 2*this->analyze_size);

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

	{
		Problem* copy_problem = problem->copy_snapshot();

		RunHelper copy_run_helper = run_helper;
		copy_run_helper.is_split = true;

		vector<ContextLayer> copy_context = context;
		if (is_branch) {
			copy_context.back().node = this->original_next_node;
		} else {
			copy_context.back().node = this->branch_next_node;
		}
		solution->scopes[0]->continue_activate(
			copy_problem,
			copy_context,
			0,
			copy_run_helper);

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = copy_problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;
		} else {
			target_val = -1.0;
		}

		this->impact = 0.001 * (run_helper.result - target_val)
			+ 0.999 * this->impact;

		delete copy_problem;
	}

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
