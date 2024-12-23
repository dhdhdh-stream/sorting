#include "branch_experiment.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "nn_helpers.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

bool BranchExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		history->instance_count++;

		run_helper.num_analyze++;

		Minesweeper* minesweeper = (Minesweeper*)problem;
		vector<double> new_input_vals;
		new_input_vals.reserve((1 + 2*this->new_analyze_size) * (1 + 2*this->new_analyze_size) + 2);
		for (int x_index = -this->new_analyze_size; x_index < this->new_analyze_size+1; x_index++) {
			for (int y_index = -this->new_analyze_size; y_index < this->new_analyze_size+1; y_index++) {
				new_input_vals.push_back(minesweeper->get_observation_helper(
						minesweeper->current_x + x_index,
						minesweeper->current_y + y_index));
			}
		}
		// new_input_vals.push_back(minesweeper->current_x);
		// new_input_vals.push_back(minesweeper->current_y);
		this->obs_histories.push_back(new_input_vals);

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

		if (this->node_context->average_instances_per_run < 1.0) {
			this->num_instances_until_target = 1;
		} else {
			uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
			this->num_instances_until_target = 1 + until_distribution(generator);
		}

		return true;
	}

	return false;
}

void BranchExperiment::train_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	if (history->instance_count > 0) {
		for (int i_index = 0; i_index < history->instance_count; i_index++) {
			double surprise = (target_val - run_helper.result) / history->instance_count;
			this->target_val_histories.push_back(surprise);
		}

		this->state_iter++;
	}

	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->new_average_score = sum_scores / num_instances;

		default_random_engine generator_copy = generator;
		shuffle(this->obs_histories.begin(), this->obs_histories.end(), generator);
		shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);

		this->new_network = new Network((1 + 2*this->new_analyze_size) * (1 + 2*this->new_analyze_size));
		// this->new_network = new Network((1 + 2*this->new_analyze_size) * (1 + 2*this->new_analyze_size) + 2);

		train_network(this->obs_histories,
					  this->target_val_histories,
					  this->new_network);

		this->obs_histories.clear();
		this->target_val_histories.clear();

		this->combined_score = 0.0;

		this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
