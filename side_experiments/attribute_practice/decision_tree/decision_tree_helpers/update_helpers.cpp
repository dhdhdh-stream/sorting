#include "decision_tree.h"

#include <iostream>

#include "constants.h"
#include "decision_tree_node.h"
#include "globals.h"
#include "network.h"

using namespace std;

const double MAX_RATIO = 0.8;
const double MIN_RATIO = 0.2;

const int NUM_PRE_FILTER = 20;
const int NUM_TRIES = 20;

void DecisionTree::update_helper(DecisionTreeNode* node) {
	vector<double> existing_predicted(node->obs_histories.size());
	for (int h_index = 0; h_index < (int)node->obs_histories.size(); h_index++) {
		existing_predicted[h_index] = node->activate(node->obs_histories[h_index],
													 node->previous_val_histories[h_index]);
	}

	uniform_int_distribution<int> train_distribution(0, DT_NUM_TRAIN_SAMPLES-1);

	Network* new_default_network = new Network(1 + node->input_indexes.size());
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = train_distribution(generator);

		vector<double> inputs(1 + node->input_indexes.size());
		inputs[0] = node->previous_val_histories[index];
		for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
			inputs[1 + i_index] = node->obs_histories[index][node->input_indexes[i_index]];
		}

		new_default_network->activate(inputs);

		double error = node->target_val_histories[index] - new_default_network->output->acti_vals[0];

		new_default_network->backprop(error);
	}

	double new_default_improvement = 0.0;
	for (int h_index = 0; h_index < DT_NUM_TEST_SAMPLES; h_index++) {
		vector<double> inputs(1 + node->input_indexes.size());
		inputs[0] = node->previous_val_histories[DT_NUM_TRAIN_SAMPLES + h_index];
		for (int i_index = 0; i_index < (int)node->input_indexes.size(); i_index++) {
			inputs[1 + i_index] = node->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index][node->input_indexes[i_index]];
		}

		new_default_network->activate(inputs);

		double new_misguess = (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - new_default_network->output->acti_vals[0])
			* (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - new_default_network->output->acti_vals[0]);
		double existing_misguess = (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[DT_NUM_TRAIN_SAMPLES + h_index])
			* (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[DT_NUM_TRAIN_SAMPLES + h_index]);

		new_default_improvement += (existing_misguess - new_misguess);
	}

	if (new_default_improvement > 0.0) {
		node->is_previous = false;
		delete node->network;
		node->network = new_default_network;
	} else {
		delete new_default_network;

		int best_obs_index;
		int best_rel_obs_index;
		int best_split_type;
		double best_split_target;
		double best_split_range;

		vector<int> best_match_input_indexes;
		Network* best_match_network = NULL;

		double best_improvement = 0.0;

		uniform_int_distribution<int> obs_distribution(0, node->obs_histories[0].size()-1);
		for (int try_index = 0; try_index < NUM_TRIES; try_index++) {
			int best_potential_split_type;
			int best_potential_obs_index;
			int best_potential_rel_obs_index;
			double best_potential_split_target;
			double best_potential_split_range;

			vector<vector<double>> best_match_obs;
			vector<double> best_match_previous_vals;
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
				vector<double> match_previous_vals;
				vector<double> match_target_vals;
				vector<vector<double>> non_match_obs;
				vector<double> non_match_previous_vals;
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
						match_previous_vals.push_back(existing_predicted[h_index]);
						match_target_vals.push_back(node->target_val_histories[h_index]);
					} else {
						non_match_obs.push_back(node->obs_histories[h_index]);
						non_match_previous_vals.push_back(existing_predicted[h_index]);
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
					best_match_previous_vals = match_previous_vals;
					best_match_target_vals = match_target_vals;

					best_diff = curr_diff;
				}

				count++;
				if (count >= NUM_PRE_FILTER) {
					break;
				}
			}

			vector<int> remaining_indexes(node->obs_histories[0].size());
			for (int i_index = 0; i_index < (int)node->obs_histories[0].size(); i_index++) {
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

				if (curr_input_indexes.size() >= DT_NODE_MAX_NUM_INPUTS) {
					break;
				}
			}

			Network* potential_eval_network = new Network(1 + curr_input_indexes.size());
			uniform_int_distribution<int> match_distribution(0, best_match_obs.size()-1);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int index = match_distribution(generator);

				vector<double> inputs(1 + curr_input_indexes.size());
				inputs[0] = best_match_previous_vals[index];
				for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
					inputs[1 + i_index] = best_match_obs[index][curr_input_indexes[i_index]];
				}

				potential_eval_network->activate(inputs);

				double error = best_match_target_vals[index] - potential_eval_network->output->acti_vals[0];

				potential_eval_network->backprop(error);
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
					vector<double> inputs(1 + curr_input_indexes.size());
					inputs[0] = existing_predicted[DT_NUM_TRAIN_SAMPLES + h_index];
					for (int i_index = 0; i_index < (int)curr_input_indexes.size(); i_index++) {
						inputs[1 + i_index] = node->obs_histories[DT_NUM_TRAIN_SAMPLES + h_index][curr_input_indexes[i_index]];
					}

					potential_eval_network->activate(inputs);

					double new_misguess = (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - potential_eval_network->output->acti_vals[0])
						* (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - potential_eval_network->output->acti_vals[0]);
					double existing_misguess = (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[DT_NUM_TRAIN_SAMPLES + h_index])
						* (node->target_val_histories[DT_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[DT_NUM_TRAIN_SAMPLES + h_index]);

					potential_improvement += (existing_misguess - new_misguess);
				}
			}

			if (potential_improvement > best_improvement) {
				best_obs_index = best_potential_obs_index;
				best_rel_obs_index = best_potential_rel_obs_index;
				best_split_type = best_potential_split_type;
				best_split_target = best_potential_split_target;
				best_split_range = best_potential_split_range;

				best_match_input_indexes = curr_input_indexes;
				if (best_match_network != NULL) {
					delete best_match_network;
				}
				best_match_network = potential_eval_network;

				best_improvement = potential_improvement;
			} else {
				delete potential_eval_network;
			}
		}

		if (best_improvement > 0.0) {
			DecisionTreeNode* new_match_node = new DecisionTreeNode();
			new_match_node->id = this->node_counter;
			this->node_counter++;
			this->nodes[new_match_node->id] = new_match_node;

			new_match_node->is_previous = false;
			new_match_node->input_indexes = best_match_input_indexes;
			new_match_node->network = best_match_network;

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

			DecisionTreeNode* new_non_match_node = new DecisionTreeNode();
			new_non_match_node->id = this->node_counter;
			this->node_counter++;
			this->nodes[new_non_match_node->id] = new_non_match_node;

			new_non_match_node->is_previous = true;
			new_non_match_node->input_indexes = node->input_indexes;
			new_non_match_node->network = NULL;

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
	node->previous_val_histories.clear();
	node->target_val_histories.clear();

	measure_helper();
}
