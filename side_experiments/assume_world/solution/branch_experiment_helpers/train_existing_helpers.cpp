#include "branch_experiment.h"

#include <algorithm>
#include <iostream>

using namespace std;

void BranchExperiment::train_existing_activate(
		Problem* problem,
		BranchExperimentHistory* history) {
	history->instance_count++;

	vector<vector<int>> input_vals(1 + 2*this->analyze_size);
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

	this->obs_histories.push_back(input_vals);
}

void BranchExperiment::train_existing_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		double final_score = (target_val - solution_set->average_score) / history->instance_count;
		this->target_val_histories.push_back(final_score);
	}

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		default_random_engine generator_copy = generator;
		shuffle(this->obs_histories.begin(), this->obs_histories.end(), generator);
		shuffle(this->target_val_histories.begin(), this->target_val_histories.end(), generator_copy);

		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / num_instances;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_score_variance += (this->target_val_histories[d_index] - this->existing_average_score) * (this->target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_standard_deviation = sqrt(sum_score_variance / num_instances);
		if (this->existing_score_standard_deviation < MIN_STANDARD_DEVIATION) {
			this->existing_score_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->existing_network = new Network(this->analyze_size);
		train_network(this->obs_histories,
					  this->target_val_histories,
					  this->existing_network);

		this->obs_histories.clear();
		this->target_val_histories.clear();

		uniform_int_distribution<int> neutral_distribution(0, 9);
		if (neutral_distribution(generator) == 0) {
			this->explore_type = EXPLORE_TYPE_NEUTRAL;
		} else {
			uniform_int_distribution<int> best_distribution(0, 1);
			if (best_distribution(generator) == 0) {
				this->explore_type = EXPLORE_TYPE_BEST;

				this->best_surprise = 0.0;
			} else {
				this->explore_type = EXPLORE_TYPE_GOOD;
			}
		}

		this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
		this->explore_iter = 0;
	}
}
