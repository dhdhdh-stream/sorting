#if defined(MDEBUG) && MDEBUG

#include "branch_node.h"

#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_set.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_activate(AbstractNode*& curr_node,
								 Problem* problem,
								 vector<ContextLayer>& context,
								 RunHelper& run_helper) {
	Minesweeper* minesweeper = (Minesweeper*)problem;

	bool is_branch;
	map<AbstractNode*, pair<int,int>>::iterator location_it;
	if (this->is_stub) {
		is_branch = false;
	} else {
		bool context_match = true;
		if (this->scope_context_ids.size() > 0) {
			if (context.size() < this->scope_context_ids.size()+1) {
				context_match = false;
			} else {
				for (int c_index = 0; c_index < (int)this->scope_context_ids.size(); c_index++) {
					if (context[context.size()-2-c_index].scope->id != this->scope_context_ids[c_index]
							|| context[context.size()-2-c_index].node->id != this->node_context_ids[c_index]) {
						context_match = false;
						break;
					}
				}
			}
		}

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

		if (!context_match || !location_match || !can_loop) {
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
				double score = this->network->output->acti_vals[0];

				if (this->verify_key != NULL) {
					cout << "this->id: " << this->id << endl;

					cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
					cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
					problem->print();

					cout << "context scope" << endl;
					for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
						cout << c_index << ": " << context[c_index].scope->id << endl;
					}
					cout << "context node" << endl;
					for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
						cout << c_index << ": " << context[c_index].node->id << endl;
					}

					if (this->verify_scores[0] != score) {
						cout << "this->verify_scores[0]: " << this->verify_scores[0] << endl;
						cout << "score: " << score << endl;

						cout << "seed: " << seed << endl;

						throw invalid_argument("branch node verify fail");
					}

					this->verify_scores.erase(this->verify_scores.begin());
				}

				if (run_helper.curr_run_seed%2 == 0) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
			}
		}
	}

	if (is_branch) {
		if (this->previous_location != NULL) {
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
	context.back().location_history[this] = {minesweeper->current_x, minesweeper->current_y};
}

#endif /* MDEBUG */