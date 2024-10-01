#include "markov_node.h"

#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void MarkovNode::result_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	Minesweeper* minesweeper = (Minesweeper*)problem;

	int iter_index = 0;
	while (true) {
		run_helper.num_analyze += (1 + (int)this->step_types.size()) * (1 + 2*MARKOV_NODE_ANALYZE_SIZE) * (1 + 2*MARKOV_NODE_ANALYZE_SIZE);

		vector<vector<double>> input_vals(1 + 2*MARKOV_NODE_ANALYZE_SIZE);
		for (int x_index = 0; x_index < 1 + 2*MARKOV_NODE_ANALYZE_SIZE; x_index++) {
			input_vals[x_index] = vector<double>(1 + 2*MARKOV_NODE_ANALYZE_SIZE);
		}
		for (int x_index = -MARKOV_NODE_ANALYZE_SIZE; x_index <= MARKOV_NODE_ANALYZE_SIZE; x_index++) {
			for (int y_index = -MARKOV_NODE_ANALYZE_SIZE; y_index <= MARKOV_NODE_ANALYZE_SIZE; y_index++) {
				input_vals[x_index + MARKOV_NODE_ANALYZE_SIZE][y_index + MARKOV_NODE_ANALYZE_SIZE]
					= minesweeper->get_observation_helper(
						minesweeper->current_x + x_index,
						minesweeper->current_y + y_index);
			}
		}

		this->halt_network->activate(input_vals);

		int best_index = -1;
		double best_val = this->halt_network->output->acti_vals[0];

		for (int o_index = 0; o_index < (int)this->step_types.size(); o_index++) {
			this->networks[o_index]->activate(input_vals);
			if (this->networks[o_index]->output->acti_vals[0] > best_val) {
				best_index = o_index;
				best_val = this->networks[o_index]->output->acti_vals[0];
			}
		}

		if (best_index == -1) {
			break;
		}

		for (int s_index = 0; s_index < (int)this->step_types[best_index].size(); s_index++) {
			switch (this->step_types[best_index][s_index]) {
			case MARKOV_STEP_TYPE_ACTION:
				problem->perform_action(this->actions[best_index][s_index]);
				break;
			case MARKOV_STEP_TYPE_SCOPE:
				this->scopes[best_index][s_index]->activate(
					problem,
					context,
					run_helper);
				break;
			}

			run_helper.num_actions++;
		}

		iter_index++;
		if (iter_index >= MARKOV_NODE_MAX_ITERS) {
			break;
		}
	}

	curr_node = this->next_node;

	run_helper.num_actions++;
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	context.back().location_history[this] = problem->get_location();
}
