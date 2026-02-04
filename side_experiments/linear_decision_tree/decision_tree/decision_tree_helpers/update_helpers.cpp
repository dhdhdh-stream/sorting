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
#include "split_node.h"

using namespace std;

const double MAX_RATIO = 0.8;
const double MIN_RATIO = 0.2;

const int NUM_SPLIT_TRIES = 20;
const int NUM_PRE_FILTER = 20;
const int NUM_TRAIN_TRIES = 20;

void find_parent(AbstractDecisionTreeNode* node,
				 DecisionTree* decision_tree,
				 SplitNode*& parent,
				 bool& is_branch) {
	for (map<int, AbstractDecisionTreeNode*>::iterator it = decision_tree->nodes.begin();
			it != decision_tree->nodes.end(); it++) {
		switch (it->second->type) {
		case DECISION_TREE_NODE_TYPE_SPLIT:
			{
				SplitNode* split_node = (SplitNode*)it->second;
				if (split_node->original_node == node) {
					parent = split_node;
					is_branch = false;
					return;
				} else if (split_node->branch_node == node) {
					parent = split_node;
					is_branch = true;
					return;
				}
			}
			break;
		}
	}
}

void DecisionTree::update_helper(EvalNode* node) {
	vector<double> existing_predicted(DT_NUM_TEST_SAMPLES);
	for (int h_index = 0; h_index < DT_NUM_TEST_SAMPLES; h_index++) {
		double sum_vals = node->constant;
		for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
			sum_vals += node->input_weights[i_index] * node->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index][node->input_indexes[i_index]];
		}

		existing_predicted[h_index] = sum_vals;
	}

	double new_default_constant;
	vector<double> new_default_input_weights;
	double new_default_improvement = 0.0;
	{
		Eigen::MatrixXd inputs(DT_NUM_TRAIN_SAMPLES, 1 + node->input_indexes.size());
		uniform_real_distribution<double> noise_distribution(-0.001, 0.001);
		/**
		 * - add some noise to prevent extremes
		 */
		for (int h_index = 0; h_index < DT_NUM_TRAIN_SAMPLES; h_index++) {
			inputs(h_index, 0) = 1.0;
			for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
				inputs(h_index, 1 + i_index) = node->obs_histories[h_index][node->input_indexes[i_index]];
			}
		}

		Eigen::VectorXd outputs(DT_NUM_TRAIN_SAMPLES);
		for (int h_index = 0; h_index < DT_NUM_TRAIN_SAMPLES; h_index++) {
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

			for (int h_index = 0; h_index < DT_NUM_TEST_SAMPLES; h_index++) {
				double sum_vals = new_default_constant;
				for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
					sum_vals += new_default_input_weights[i_index] * node->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index][node->input_indexes[i_index]];
				}

				double new_misguess = (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - sum_vals)
					* (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - sum_vals);
				double existing_misguess = (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index])
					* (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index]);

				new_default_improvement += (existing_misguess - new_misguess);
			}
		}
	}

	if (new_default_improvement > 0.0) {
		node->constant = new_default_constant;
		node->input_weights = new_default_input_weights;
	} else {
		int best_obs_index;
		int best_rel_obs_index;
		int best_split_type;
		double best_split_target;
		double best_split_range;

		double best_constant;
		vector<int> best_input_indexes;
		vector<double> best_input_weights;

		double best_improvement = 0.0;

		uniform_int_distribution<int> obs_distribution(0, node->obs_histories[0].size()-1);
		uniform_int_distribution<int> train_distribution(0, DT_NUM_TRAIN_SAMPLES-1);
		for (int split_index = 0; split_index < NUM_SPLIT_TRIES; split_index++) {
			int best_potential_split_type;
			int best_potential_obs_index;
			int best_potential_rel_obs_index;
			double best_potential_split_target;
			double best_potential_split_range;

			vector<vector<double>> best_match_obs;
			vector<double> best_match_target_vals;

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
				for (int h_index = 0; h_index < DT_NUM_TRAIN_SAMPLES; h_index++) {
					bool is_match = is_match_helper(node->obs_histories[h_index],
													potential_obs_index,
													potential_rel_obs_index,
													potential_split_type,
													potential_split_target,
													potential_split_range);

					if (is_match) {
						match_obs.push_back(node->obs_histories[h_index]);
						match_target_vals.push_back(node->target_val_histories[h_index]);
					} else {
						non_match_obs.push_back(node->obs_histories[h_index]);
						non_match_target_vals.push_back(node->target_val_histories[h_index]);
					}
				}

				double max_num_samples = MAX_RATIO * (double)DT_NUM_TRAIN_SAMPLES;
				double min_num_samples = MIN_RATIO * (double)DT_NUM_TRAIN_SAMPLES;
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

					best_diff = curr_diff;
				}

				count++;
				if (count >= NUM_PRE_FILTER) {
					break;
				}
			}

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

					if (curr_input_indexes.size() >= EVAL_NODE_NUM_INPUTS) {
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
				for (int h_index = 0; h_index < DT_NUM_TEST_SAMPLES; h_index++) {
					bool is_match = is_match_helper(node->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index],
													best_potential_obs_index,
													best_potential_rel_obs_index,
													best_potential_split_type,
													best_potential_split_target,
													best_potential_split_range);

					if (is_match) {
						double sum_vals = curr_constant;
						for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
							sum_vals += curr_input_weights[i_index] * node->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index][curr_input_indexes[i_index]];
						}

						double new_misguess = (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - sum_vals)
							* (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - sum_vals);
						double existing_misguess = (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index])
							* (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index]);

						potential_improvement += (existing_misguess - new_misguess);
					}
				}

				if (potential_improvement > best_improvement) {
					best_obs_index = best_potential_obs_index;
					best_rel_obs_index = best_potential_rel_obs_index;
					best_split_type = best_potential_split_type;
					best_split_target = best_potential_split_target;
					best_split_range = best_potential_split_range;

					best_constant = curr_constant;
					best_input_indexes = curr_input_indexes;
					best_input_weights = curr_input_weights;

					best_improvement = potential_improvement;
				}
			}
		}

		if (best_improvement > 0.0) {
			// temp
			cout << "best_improvement: " << best_improvement << endl;

			SplitNode* parent = NULL;
			bool is_branch;
			find_parent(node,
						this,
						parent,
						is_branch);

			EvalNode* new_eval_node = new EvalNode();
			new_eval_node->id = this->node_counter;
			this->node_counter++;
			this->nodes[new_eval_node->id] = new_eval_node;

			new_eval_node->constant = best_constant;
			new_eval_node->input_indexes = best_input_indexes;
			new_eval_node->input_weights = best_input_weights;

			SplitNode* new_split_node = new SplitNode();
			new_split_node->id = this->node_counter;
			this->node_counter++;
			this->nodes[new_split_node->id] = new_split_node;

			new_split_node->obs_index = best_obs_index;
			new_split_node->rel_obs_index = best_rel_obs_index;
			new_split_node->split_type = best_split_type;
			new_split_node->split_target = best_split_target;
			new_split_node->split_range = best_split_range;

			new_split_node->original_node_id = node->id;
			new_split_node->original_node = node;
			new_split_node->branch_node_id = new_eval_node->id;
			new_split_node->branch_node = new_eval_node;

			// temp
			cout << "node->id: " << node->id << endl;
			cout << "parent: " << parent << endl;
			if (parent != NULL) {
				cout << "parent->id: " << parent->id << endl;
			}

			if (parent == NULL) {
				this->root = new_split_node;
			} else {
				if (is_branch) {
					parent->branch_node_id = new_split_node->id;
					parent->branch_node = new_split_node;
				} else {
					parent->original_node_id = new_split_node->id;
					parent->original_node = new_split_node;
				}
			}
		}
	}

	node->obs_histories.clear();
	node->target_val_histories.clear();

	measure_helper();
}
