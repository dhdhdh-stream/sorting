#include "branch_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "new_action_experiment.h"
#include "scope.h"
#include "solution.h"
#include "solution_set.h"
#include "utilities.h"

using namespace std;

void BranchNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper) {
	Minesweeper* minesweeper = (Minesweeper*)problem;

	bool is_branch;
	map<AbstractNode*, pair<int,int>>::iterator location_it;
	if (this->is_stub) {
		is_branch = false;
	} else {
		bool can_loop = true;
		if (this->is_loop) {
			set<AbstractNode*>::iterator loop_start_it = context.back().loop_nodes_seen.find(this);
			if (loop_start_it != context.back().loop_nodes_seen.end()) {
				can_loop = false;

				context.back().loop_nodes_seen.erase(loop_start_it);
			}
		}

		bool location_match = true;
		if (this->previous_location != NULL) {
			location_it = context.back().location_history.find(this->previous_location);
			if (location_it == context.back().location_history.end()) {
				location_match = false;
			}
		}

		if (!location_match || !can_loop) {
			is_branch = false;
		} else {
			if (this->analyze_size == -1) {
				is_branch = true;
			} else {
				run_helper.num_analyze += (1 + 2*this->analyze_size) * (1 + 2*this->analyze_size);

				vector<vector<double>> input_vals(1 + 2*this->analyze_size);
				for (int x_index = 0; x_index < 1 + 2*this->analyze_size; x_index++) {
					input_vals[x_index] = vector<double>(1 + 2*this->analyze_size);
				}

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
			}
		}
	}

	if (is_branch) {
		if (this->previous_location_id != -1) {
			minesweeper->current_x = location_it->second.first;
			minesweeper->current_y = location_it->second.second;
		}

		if (this->is_loop) {
			context.back().loop_nodes_seen.insert(this);
		}

		curr_node = this->branch_next_node;
	} else {
		curr_node = this->original_next_node;
	}

	run_helper.num_actions++;
	Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
	if (run_helper.num_actions > solution->num_actions_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	if (run_helper.experiments_seen_order.size() == 0) {
		run_helper.nodes_seen.insert({this, is_branch});
	} else if (run_helper.experiment_histories.size() == 1
			&& run_helper.experiment_histories.back()->experiment == this->parent->new_action_experiment) {
		context.back().nodes_seen.push_back({this, is_branch});
	}
	context.back().location_history[this] = {minesweeper->current_x, minesweeper->current_y};

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
