#include "sum_tree.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "constants.h"
#include "globals.h"
#include "sum_tree_node.h"

using namespace std;

const int NUM_INIT_TRIES = 20;

void SumTree::init_helper() {
	double best_constant = 0.0;
	vector<int> best_input_indexes;
	vector<double> best_input_weights;

	double best_sum_misguess = numeric_limits<double>::max();

	uniform_int_distribution<int> input_distribution(0, this->obs_histories[0].size()-1);
	for (int try_index = 0; try_index < NUM_INIT_TRIES; try_index++) {
		vector<int> remaining_indexes(this->obs_histories[0].size());
		for (int i_index = 0; i_index < (int)this->obs_histories[0].size(); i_index++) {
			remaining_indexes[i_index] = i_index;
		}

		vector<int> curr_input_indexes;
		while (true) {
			if (remaining_indexes.size() == 0) {
				break;
			}

			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int index = distribution(generator);

			curr_input_indexes.push_back(remaining_indexes[index]);

			remaining_indexes.erase(remaining_indexes.begin() + index);

			if (curr_input_indexes.size() >= SUM_TREE_NODE_MAX_NUM_INPUTS) {
				break;
			}
		}

		Eigen::MatrixXd inputs(ST_NUM_TRAIN_SAMPLES, 1 + curr_input_indexes.size());
		uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
		/**
		 * - add some noise to prevent extremes
		 */
		for (int h_index = 0; h_index < ST_NUM_TRAIN_SAMPLES; h_index++) {
			inputs(h_index, 0) = 1.0;
			for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
				inputs(h_index, 1 + i_index) = this->obs_histories[h_index][curr_input_indexes[i_index]];
			}
		}

		Eigen::VectorXd outputs(ST_NUM_TRAIN_SAMPLES);
		for (int h_index = 0; h_index < ST_NUM_TRAIN_SAMPLES; h_index++) {
			outputs(h_index) = this->target_val_histories[h_index];
		}

		Eigen::VectorXd weights;
		try {
			weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			continue;
		}

		#if defined(MDEBUG) && MDEBUG
		#else
		bool limit_exceeded = false;
		if (abs(weights(0)) > REGRESSION_WEIGHT_LIMIT) {
			cout << "abs(weights(0)): " << abs(weights(0)) << endl;
			limit_exceeded = true;
		}
		for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
			if (abs(weights(1 + i_index)) > REGRESSION_WEIGHT_LIMIT) {
				cout << "abs(weights(1 + i_index)): " << abs(weights(1 + i_index)) << endl;
				limit_exceeded = true;
			}
		}
		if (limit_exceeded) {
			continue;
		}
		#endif /* MDEBUG */

		double curr_constant = weights(0);
		vector<double> curr_input_weights(curr_input_indexes.size());
		for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
			curr_input_weights[i_index] = weights(1 + i_index);
		}

		double curr_sum_misguess = 0.0;
		for (int h_index = 0; h_index < ST_NUM_TEST_SAMPLES; h_index++) {
			double sum_vals = curr_constant;
			for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
				sum_vals += curr_input_weights[i_index] * this->obs_histories[ST_NUM_TRAIN_SAMPLES + h_index][curr_input_indexes[i_index]];
			}

			curr_sum_misguess += (this->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - sum_vals)
				* (this->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - sum_vals);
		}

		if (curr_sum_misguess < best_sum_misguess) {
			best_constant = curr_constant;
			best_input_indexes = curr_input_indexes;
			best_input_weights = curr_input_weights;

			best_sum_misguess = curr_sum_misguess;
		}
	}

	SumTreeNode* new_node = new SumTreeNode();
	new_node->id = this->node_counter;
	this->node_counter++;
	this->nodes[new_node->id] = new_node;

	new_node->constant = best_constant;
	new_node->input_indexes = best_input_indexes;
	new_node->input_weights = best_input_weights;
	new_node->previous_weight = 0.0;

	new_node->has_split = false;
	new_node->obs_index = -1;
	new_node->rel_obs_index = -1;
	new_node->split_type = -1;
	new_node->split_target = 0.0;
	new_node->split_range = 0.0;

	new_node->original_node_id = -1;
	new_node->original_node = NULL;
	new_node->branch_node_id = -1;
	new_node->branch_node = NULL;

	this->root = new_node;

	measure_helper();
}
