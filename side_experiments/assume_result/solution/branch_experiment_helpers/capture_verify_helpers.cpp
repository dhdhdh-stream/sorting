#if defined(MDEBUG) && MDEBUG

#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "minesweeper.h"
#include "network.h"
#include "problem.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::capture_verify_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   vector<ContextLayer>& context,
											   RunHelper& run_helper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	run_helper.num_actions++;

	if (context.back().branch_node_ancestors.find(this->branch_node) != context.back().branch_node_ancestors.end()) {
		return false;
	}

	bool can_loop = true;
	if (this->curr_is_loop) {
		set<AbstractNode*>::iterator loop_start_it = context.back().loop_nodes_seen.find(this->branch_node);
		if (loop_start_it != context.back().loop_nodes_seen.end()) {
			can_loop = false;

			context.back().loop_nodes_seen.erase(loop_start_it);
		}
	}

	bool location_match = true;
	map<AbstractNode*, pair<int,int>>::iterator location_it;
	if (this->curr_previous_location != NULL) {
		location_it = context.back().location_history.find(this->curr_previous_location);
		if (location_it == context.back().location_history.end()) {
			location_match = false;
		}
	}

	if (location_match && can_loop) {
		run_helper.num_analyze += (1 + 2*this->new_analyze_size) * (1 + 2*this->new_analyze_size);

		vector<vector<double>> input_vals(1 + 2*this->new_analyze_size);
		for (int x_index = 0; x_index < 1 + 2*this->new_analyze_size; x_index++) {
			input_vals[x_index] = vector<double>(1 + 2*this->new_analyze_size);
		}

		Minesweeper* minesweeper = (Minesweeper*)problem;
		for (int x_index = -this->new_analyze_size; x_index < this->new_analyze_size+1; x_index++) {
			for (int y_index = -this->new_analyze_size; y_index < this->new_analyze_size+1; y_index++) {
				input_vals[x_index + this->new_analyze_size][y_index + this->new_analyze_size]
					= minesweeper->get_observation_helper(
						minesweeper->current_x + x_index,
						minesweeper->current_y + y_index);
			}
		}
		this->new_network->activate(input_vals);
		double new_predicted_score = this->new_network->output->acti_vals[0];

		this->verify_scores.push_back(new_predicted_score);

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

		bool decision_is_branch;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

		cout << "decision_is_branch: " << decision_is_branch << endl;

		if (decision_is_branch) {
			context.back().branch_nodes_seen.insert(this->branch_node);

			if (this->curr_previous_location != NULL) {
				Minesweeper* minesweeper = (Minesweeper*)problem;
				minesweeper->current_x = location_it->second.first;
				minesweeper->current_y = location_it->second.second;
			}

			if (this->curr_is_loop) {
				context.back().loop_nodes_seen.insert(this->branch_node);
			}

			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else if (this->best_step_types[0] == STEP_TYPE_SCOPE) {
					curr_node = this->best_scopes[0];
				} else {
					curr_node = this->best_returns[0];
				}
			}

			return true;
		}
	}

	return false;
}

void BranchExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	}
}

#endif /* MDEBUG */