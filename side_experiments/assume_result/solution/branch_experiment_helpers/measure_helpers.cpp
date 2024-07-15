#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_set.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::measure_activate(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	run_helper.num_actions++;

	if (run_helper.branch_node_ancestors.find(this->branch_node) != run_helper.branch_node_ancestors.end()) {
		return false;
	}

	bool location_match = true;
	map<AbstractNode*, pair<int,int>>::iterator location_it;
	if (this->curr_previous_location != NULL) {
		location_it = context.back().location_history.find(this->curr_previous_location);
		if (location_it == context.back().location_history.end()) {
			location_match = false;
		}
	}
	if (location_match) {
		history->instance_count++;

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

		#if defined(MDEBUG) && MDEBUG
		bool decision_is_branch;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		bool decision_is_branch = this->new_network->output->acti_vals[0] >= 0.0;
		#endif /* MDEBUG */

		if (decision_is_branch) {
			if (this->curr_previous_location != NULL) {
				Minesweeper* minesweeper = (Minesweeper*)problem;
				minesweeper->current_x = location_it->second.first;
				minesweeper->current_y = location_it->second.second;
			}

			run_helper.branch_node_ancestors.insert(this->branch_node);
			context.back().branch_nodes_seen.push_back(this->branch_node);

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

void BranchExperiment::measure_backprop(
		double target_val,
		RunHelper& run_helper) {
	if (run_helper.exceeded_limit) {
		this->result = EXPERIMENT_RESULT_FAIL;
	} else {
		BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

		for (int i_index = 0; i_index < history->instance_count; i_index++) {
			double final_score = (target_val - run_helper.result) / history->instance_count;
			this->combined_score += final_score;
			this->sub_state_iter++;
		}

		this->state_iter++;
		if (this->sub_state_iter >= NUM_DATAPOINTS
				&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
			this->combined_score /= this->sub_state_iter;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->combined_score > 0.0) {
			#endif /* MDEBUG */
				cout << "BranchExperiment" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index]->action.move;
					} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
						cout << " E";
					} else {
						cout << " R";
					}
				}
				cout << endl;

				if (this->best_exit_next_node == NULL) {
					cout << "this->best_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				}

				cout << "this->combined_score: " << this->combined_score << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
