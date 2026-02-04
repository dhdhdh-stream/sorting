#include "decision_tree.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "constants.h"
#include "eval_node.h"
#include "globals.h"
#include "network.h"

using namespace std;

const int NUM_INIT_TRIES = 20;

void DecisionTree::init_helper() {
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

			if (curr_input_indexes.size() >= EVAL_NODE_NUM_INPUTS) {
				break;
			}
		}

		Eigen::MatrixXd inputs(DT_NUM_TRAIN_SAMPLES, 1 + curr_input_indexes.size());
		uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
		/**
		 * - add some noise to prevent extremes
		 */
		for (int h_index = 0; h_index < DT_NUM_TRAIN_SAMPLES; h_index++) {
			inputs(h_index, 0) = 1.0;
			for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
				inputs(h_index, 1 + i_index) = this->obs_histories[h_index][curr_input_indexes[i_index]];
			}
		}

		Eigen::VectorXd outputs(DT_NUM_TRAIN_SAMPLES);
		for (int h_index = 0; h_index < DT_NUM_TRAIN_SAMPLES; h_index++) {
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
		for (int h_index = 0; h_index < DT_NUM_TEST_SAMPLES; h_index++) {
			double sum_vals = curr_constant;
			for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
				sum_vals += curr_input_weights[i_index] * this->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index][curr_input_indexes[i_index]];
			}

			curr_sum_misguess += (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - sum_vals)
				* (this->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - sum_vals);
		}

		if (curr_sum_misguess < best_sum_misguess) {
			best_constant = curr_constant;
			best_input_indexes = curr_input_indexes;
			best_input_weights = curr_input_weights;

			best_sum_misguess = curr_sum_misguess;
		}
	}

	EvalNode* new_eval_node = new EvalNode();
	new_eval_node->id = this->node_counter;
	this->node_counter++;
	this->nodes[new_eval_node->id] = new_eval_node;

	new_eval_node->constant = best_constant;
	new_eval_node->input_indexes = best_input_indexes;
	new_eval_node->input_weights = best_input_weights;

	this->root = new_eval_node;

	measure_helper();
}
