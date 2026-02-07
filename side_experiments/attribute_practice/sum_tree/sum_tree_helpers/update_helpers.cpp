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

const double MAX_RATIO = 0.8;
const double MIN_RATIO = 0.2;

const int NUM_SPLIT_TRIES = 20;
const int NUM_PRE_FILTER = 20;
const int NUM_TRAIN_TRIES = 20;

void SumTree::update_helper(SumTreeNode* node) {
	vector<double> existing_predicted(ST_NUM_TEST_SAMPLES);
	for (int h_index = 0; h_index < ST_NUM_TEST_SAMPLES; h_index++) {
		double sum_vals = node->constant;
		for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
			sum_vals += node->input_weights[i_index] * node->obs_histories[ST_NUM_TRAIN_SAMPLES + h_index][node->input_indexes[i_index]];
		}

		existing_predicted[h_index] = sum_vals;
	}

	double new_default_constant;
	vector<double> new_default_input_weights;
	double new_default_improvement = 0.0;
	{
		Eigen::MatrixXd inputs(ST_NUM_TRAIN_SAMPLES, 1 + node->input_indexes.size());
		uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
		/**
		 * - add some noise to prevent extremes
		 */
		for (int h_index = 0; h_index < ST_NUM_TRAIN_SAMPLES; h_index++) {
			inputs(h_index, 0) = 1.0;
			for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
				inputs(h_index, 1 + i_index) = node->obs_histories[h_index][node->input_indexes[i_index]];
			}
		}

		Eigen::VectorXd outputs(ST_NUM_TRAIN_SAMPLES);
		for (int h_index = 0; h_index < ST_NUM_TRAIN_SAMPLES; h_index++) {
			outputs(h_index) = node->target_val_histories[h_index];
		}

		bool train_success;
		Eigen::VectorXd weights;
		try {
			weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			train_success = true;
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			train_success = false;
		}

		bool limit_exceeded = false;
		#if defined(MDEBUG) && MDEBUG
		#else
		if (train_success) {
			if (abs(weights(0)) > REGRESSION_WEIGHT_LIMIT) {
				cout << "abs(weights(0)): " << abs(weights(0)) << endl;
				limit_exceeded = true;
			}
			for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
				if (abs(weights(1 + i_index)) > REGRESSION_WEIGHT_LIMIT) {
					cout << "abs(weights(1 + i_index)): " << abs(weights(1 + i_index)) << endl;
					limit_exceeded = true;
				}
			}
		}
		#endif /* MDEBUG */

		if (train_success && !limit_exceeded) {
			new_default_constant = weights(0);
			new_default_input_weights = vector<double>(node->input_indexes.size());
			for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
				new_default_input_weights[i_index] = weights(1 + i_index);
			}

			for (int h_index = 0; h_index < ST_NUM_TEST_SAMPLES; h_index++) {
				double sum_vals = new_default_constant;
				for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
					sum_vals += new_default_input_weights[i_index] * node->obs_histories[ST_NUM_TRAIN_SAMPLES + h_index][node->input_indexes[i_index]];
				}

				double new_misguess = (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - sum_vals)
					* (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - sum_vals);
				double existing_misguess = (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index])
					* (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index]);

				new_default_improvement += (existing_misguess - new_misguess);
			}
		}
	}

	if (new_default_improvement > 0.0) {
		node->constant = new_default_constant;
		node->input_weights = new_default_input_weights;
	} else {
		vector<double> remaining_vals(ST_NUM_TRAIN_SAMPLES);
		for (int h_index = 0; h_index < ST_NUM_TRAIN_SAMPLES; h_index++) {
			double sum_vals = node->constant;
			for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
				sum_vals += node->input_weights[i_index] * node->obs_histories[h_index][node->input_indexes[i_index]];
			}

			remaining_vals[h_index] = node->target_val_histories[h_index] - sum_vals;
		}

		int best_obs_index;
		int best_rel_obs_index;
		int best_split_type;
		double best_split_target;
		double best_split_range;

		double best_match_constant;
		vector<int> best_match_input_indexes;
		vector<double> best_match_input_weights;

		double best_non_match_constant;
		vector<int> best_non_match_input_indexes;
		vector<double> best_non_match_input_weights;

		double best_improvement = 0.0;

		uniform_int_distribution<int> obs_distribution(0, node->obs_histories[0].size()-1);
		uniform_int_distribution<int> train_distribution(0, ST_NUM_TRAIN_SAMPLES-1);
		for (int split_index = 0; split_index < NUM_SPLIT_TRIES; split_index++) {
			int best_potential_split_type;
			int best_potential_obs_index;
			int best_potential_rel_obs_index;
			double best_potential_split_target;
			double best_potential_split_range;

			vector<vector<double>> best_match_obs;
			vector<double> best_match_target_vals;
			vector<vector<double>> best_non_match_obs;
			vector<double> best_non_match_target_vals;

			double best_diff = 0.0;
			int count = 0;
			while (true) {
				int potential_split_type;
				if (node->obs_histories[0].size() <= 1) {
					uniform_int_distribution<int> type_distribution(0, 7);
					potential_split_type = type_distribution(generator);
				} else {
					uniform_int_distribution<int> type_distribution(0, 13);
					potential_split_type = type_distribution(generator);
				}

				int potential_obs_index;
				int potential_rel_obs_index;
				double potential_split_target;
				double potential_split_range;
				switch (potential_split_type) {
				case SPLIT_TYPE_GREATER:
				case SPLIT_TYPE_GREATER_EQUAL:
				case SPLIT_TYPE_LESSER:
				case SPLIT_TYPE_LESSER_EQUAL:
					{
						potential_obs_index = obs_distribution(generator);
						potential_rel_obs_index = -1;
						int potential_index = train_distribution(generator);
						potential_split_target = node->obs_histories[potential_index][potential_obs_index];
						potential_split_range = 0.0;
					}
					break;
				case SPLIT_TYPE_WITHIN:
				case SPLIT_TYPE_WITHIN_EQUAL:
				case SPLIT_TYPE_WITHOUT:
				case SPLIT_TYPE_WITHOUT_EQUAL:
					{
						potential_obs_index = obs_distribution(generator);
						potential_rel_obs_index = -1;
						int potential_index = train_distribution(generator);
						potential_split_target = node->obs_histories[potential_index][potential_obs_index];
						int potential_rel_index = train_distribution(generator);
						potential_split_range = abs(potential_split_target - node->obs_histories[potential_rel_index][potential_obs_index]);
					}
					break;
				case SPLIT_TYPE_REL_GREATER:
				case SPLIT_TYPE_REL_GREATER_EQUAL:
					{
						potential_obs_index = obs_distribution(generator);
						while (true) {
							potential_rel_obs_index = obs_distribution(generator);
							if (potential_rel_obs_index != potential_obs_index) {
								break;
							}
						}
						int potential_index = train_distribution(generator);
						potential_split_target = node->obs_histories[potential_index][potential_obs_index] - node->obs_histories[potential_index][potential_rel_obs_index];
						potential_split_range = 0.0;
					}
					break;
				case SPLIT_TYPE_REL_WITHIN:
				case SPLIT_TYPE_REL_WITHIN_EQUAL:
				case SPLIT_TYPE_REL_WITHOUT:
				case SPLIT_TYPE_REL_WITHOUT_EQUAL:
					{
						potential_obs_index = obs_distribution(generator);
						while (true) {
							potential_rel_obs_index = obs_distribution(generator);
							if (potential_rel_obs_index != potential_obs_index) {
								break;
							}
						}
						int potential_index = train_distribution(generator);
						potential_split_target = node->obs_histories[potential_index][potential_obs_index] - node->obs_histories[potential_index][potential_rel_obs_index];
						int potential_rel_index = train_distribution(generator);
						potential_split_range = abs(potential_split_target - (node->obs_histories[potential_rel_index][potential_obs_index] - node->obs_histories[potential_rel_index][potential_rel_obs_index]));
					}
					break;
				}

				vector<vector<double>> match_obs;
				vector<double> match_target_vals;
				vector<vector<double>> non_match_obs;
				vector<double> non_match_target_vals;
				for (int h_index = 0; h_index < ST_NUM_TRAIN_SAMPLES; h_index++) {
					bool is_match = is_match_helper(node->obs_histories[h_index],
													potential_obs_index,
													potential_rel_obs_index,
													potential_split_type,
													potential_split_target,
													potential_split_range);

					if (is_match) {
						match_obs.push_back(node->obs_histories[h_index]);
						match_target_vals.push_back(remaining_vals[h_index]);
					} else {
						non_match_obs.push_back(node->obs_histories[h_index]);
						non_match_target_vals.push_back(remaining_vals[h_index]);
					}
				}

				double max_num_samples = MAX_RATIO * (double)ST_NUM_TRAIN_SAMPLES;
				double min_num_samples = MIN_RATIO * (double)ST_NUM_TRAIN_SAMPLES;
				if (match_obs.size() < min_num_samples || match_obs.size() > max_num_samples) {
					continue;
				}

				double match_sum_vals = 0.0;
				for (int i_index = 0; i_index < (int)match_target_vals.size(); i_index++) {
					match_sum_vals += match_target_vals[i_index];
				}
				double match_val_average = match_sum_vals / (double)match_target_vals.size();

				double non_match_sum_vals = 0.0;
				for (int i_index = 0; i_index < (int)non_match_target_vals.size(); i_index++) {
					non_match_sum_vals += non_match_target_vals[i_index];
				}
				double non_match_val_average = non_match_sum_vals / (double)non_match_target_vals.size();

				double curr_diff = abs(match_val_average - non_match_val_average);
				if (curr_diff > best_diff) {
					best_potential_split_type = potential_split_type;
					best_potential_obs_index = potential_obs_index;
					best_potential_rel_obs_index = potential_rel_obs_index;
					best_potential_split_target = potential_split_target;
					best_potential_split_range = potential_split_range;

					best_match_obs = match_obs;
					best_match_target_vals = match_target_vals;
					best_non_match_obs = non_match_obs;
					best_non_match_target_vals = non_match_target_vals;

					best_diff = curr_diff;
				}

				count++;
				if (count >= NUM_PRE_FILTER) {
					break;
				}
			}

			double best_potential_match_constant;
			vector<int> best_potential_match_input_indexes;
			vector<double> best_potential_match_input_weights;

			double best_potential_match_improvement = 0.0;

			for (int train_index = 0; train_index < NUM_TRAIN_TRIES; train_index++) {
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

				Eigen::MatrixXd inputs(best_match_obs.size(), 1 + curr_input_indexes.size());
				uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
				/**
				 * - add some noise to prevent extremes
				 */
				for (int h_index = 0; h_index < (int)best_match_obs.size(); h_index++) {
					inputs(h_index, 0) = 1.0;
					for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
						inputs(h_index, 1 + i_index) = best_match_obs[h_index][curr_input_indexes[i_index]];
					}
				}

				Eigen::VectorXd outputs(best_match_obs.size());
				for (int h_index = 0; h_index < (int)best_match_obs.size(); h_index++) {
					outputs(h_index) = best_match_target_vals[h_index];
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

				double potential_improvement = 0.0;
				for (int h_index = 0; h_index < ST_NUM_TEST_SAMPLES; h_index++) {
					bool is_match = is_match_helper(node->obs_histories[ST_NUM_TRAIN_SAMPLES + h_index],
													best_potential_obs_index,
													best_potential_rel_obs_index,
													best_potential_split_type,
													best_potential_split_target,
													best_potential_split_range);

					if (is_match) {
						double sum_vals = existing_predicted[h_index] + curr_constant;
						for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
							sum_vals += curr_input_weights[i_index] * node->obs_histories[ST_NUM_TRAIN_SAMPLES + h_index][curr_input_indexes[i_index]];
						}

						double new_misguess = (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - sum_vals)
							* (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - sum_vals);
						double existing_misguess = (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index])
							* (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index]);

						potential_improvement += (existing_misguess - new_misguess);
					}
				}

				if (potential_improvement > best_potential_match_improvement) {
					best_potential_match_constant = curr_constant;
					best_potential_match_input_indexes = curr_input_indexes;
					best_potential_match_input_weights = curr_input_weights;

					best_potential_match_improvement = potential_improvement;
				}
			}

			double best_potential_non_match_constant;
			vector<int> best_potential_non_match_input_indexes;
			vector<double> best_potential_non_match_input_weights;

			double best_potential_non_match_improvement = 0.0;

			for (int train_index = 0; train_index < NUM_TRAIN_TRIES; train_index++) {
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

				Eigen::MatrixXd inputs(best_non_match_obs.size(), 1 + curr_input_indexes.size());
				uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
				/**
				 * - add some noise to prevent extremes
				 */
				for (int h_index = 0; h_index < (int)best_non_match_obs.size(); h_index++) {
					inputs(h_index, 0) = 1.0;
					for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
						inputs(h_index, 1 + i_index) = best_non_match_obs[h_index][curr_input_indexes[i_index]];
					}
				}

				Eigen::VectorXd outputs(best_non_match_obs.size());
				for (int h_index = 0; h_index < (int)best_non_match_obs.size(); h_index++) {
					outputs(h_index) = best_non_match_target_vals[h_index];
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

				double potential_improvement = 0.0;
				for (int h_index = 0; h_index < ST_NUM_TEST_SAMPLES; h_index++) {
					bool is_match = is_match_helper(node->obs_histories[ST_NUM_TRAIN_SAMPLES + h_index],
													best_potential_obs_index,
													best_potential_rel_obs_index,
													best_potential_split_type,
													best_potential_split_target,
													best_potential_split_range);

					if (!is_match) {
						double sum_vals = existing_predicted[h_index] + curr_constant;
						for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
							sum_vals += curr_input_weights[i_index] * node->obs_histories[ST_NUM_TRAIN_SAMPLES + h_index][curr_input_indexes[i_index]];
						}

						double new_misguess = (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - sum_vals)
							* (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - sum_vals);
						double existing_misguess = (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index])
							* (node->target_val_histories[ST_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index]);

						potential_improvement += (existing_misguess - new_misguess);
					}
				}

				if (potential_improvement > best_potential_non_match_improvement) {
					best_potential_non_match_constant = curr_constant;
					best_potential_non_match_input_indexes = curr_input_indexes;
					best_potential_non_match_input_weights = curr_input_weights;

					best_potential_non_match_improvement = potential_improvement;
				}
			}

			if (best_potential_match_improvement > 0.0
					&& best_potential_non_match_improvement > 0.0
					&& (best_potential_match_improvement + best_potential_non_match_improvement) > best_improvement) {
				best_obs_index = best_potential_obs_index;
				best_rel_obs_index = best_potential_rel_obs_index;
				best_split_type = best_potential_split_type;
				best_split_target = best_potential_split_target;
				best_split_range = best_potential_split_range;

				best_match_constant = best_potential_match_constant;
				best_match_input_indexes = best_potential_match_input_indexes;
				best_match_input_weights = best_potential_match_input_weights;

				best_non_match_constant = best_potential_non_match_constant;
				best_non_match_input_indexes = best_potential_non_match_input_indexes;
				best_non_match_input_weights = best_potential_non_match_input_weights;

				best_improvement = best_potential_match_improvement + best_potential_non_match_improvement;
			}
		}

		if (best_improvement > 0.0) {
			SumTreeNode* new_match_node = new SumTreeNode();
			new_match_node->id = this->node_counter;
			this->node_counter++;
			this->nodes[new_match_node->id] = new_match_node;

			new_match_node->constant = best_match_constant;
			new_match_node->input_indexes = best_match_input_indexes;
			new_match_node->input_weights = best_match_input_weights;

			new_match_node->has_split = false;
			new_match_node->obs_index = -1;
			new_match_node->rel_obs_index = -1;
			new_match_node->split_type = -1;
			new_match_node->split_target = 0.0;
			new_match_node->split_range = 0.0;

			new_match_node->original_node_id = -1;
			new_match_node->original_node = NULL;
			new_match_node->branch_node_id = -1;
			new_match_node->branch_node = NULL;

			SumTreeNode* new_non_match_node = new SumTreeNode();
			new_non_match_node->id = this->node_counter;
			this->node_counter++;
			this->nodes[new_non_match_node->id] = new_non_match_node;

			new_non_match_node->constant = best_non_match_constant;
			new_non_match_node->input_indexes = best_non_match_input_indexes;
			new_non_match_node->input_weights = best_non_match_input_weights;

			new_non_match_node->has_split = false;
			new_non_match_node->obs_index = -1;
			new_non_match_node->rel_obs_index = -1;
			new_non_match_node->split_type = -1;
			new_non_match_node->split_target = 0.0;
			new_non_match_node->split_range = 0.0;

			new_non_match_node->original_node_id = -1;
			new_non_match_node->original_node = NULL;
			new_non_match_node->branch_node_id = -1;
			new_non_match_node->branch_node = NULL;

			node->has_split = true;
			node->obs_index = best_obs_index;
			node->rel_obs_index = best_rel_obs_index;
			node->split_type = best_split_type;
			node->split_target = best_split_target;
			node->split_range = best_split_range;

			node->original_node_id = new_non_match_node->id;
			node->original_node = new_non_match_node;
			node->branch_node_id = new_match_node->id;
			node->branch_node = new_match_node;
		}
	}

	node->obs_histories.clear();
	node->target_val_histories.clear();

	measure_helper();
}
