#include "decision_tree.h"

#include <iostream>

#include "constants.h"
#include "eval_node.h"
#include "globals.h"
#include "network.h"
#include "split_node.h"

using namespace std;

const double MAX_RATIO = 0.8;
const double MIN_RATIO = 0.2;

const int NUM_PRE_FILTER = 40;
const int NUM_TRIES = 20;

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
	// temp
	cout << "update_helper" << endl;

	vector<double> existing_predicted(EVAL_NUM_TEST_SAMPLES);
	for (int i_index = 0; i_index < EVAL_NUM_TEST_SAMPLES; i_index++) {
		node->network->activate(node->obs_histories[EVAL_NUM_TRAIN_SAMPLES + i_index]);
		existing_predicted[i_index] = node->network->output->acti_vals[0];
	}

	int best_obs_index;
	int best_rel_obs_index;
	int best_split_type;
	double best_split_target;
	double best_split_range;
	Network* best_eval_network = NULL;
	double best_improvement = 0.0;

	uniform_int_distribution<int> obs_distribution(0, node->obs_histories[0].size()-1);
	uniform_int_distribution<int> train_distribution(0, EVAL_NUM_TRAIN_SAMPLES-1);
	for (int try_index = 0; try_index < NUM_TRIES; try_index++) {
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
			for (int h_index = 0; h_index < EVAL_NUM_TRAIN_SAMPLES; h_index++) {
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

			double max_num_samples = MAX_RATIO * (double)EVAL_NUM_TRAIN_SAMPLES;
			double min_num_samples = MIN_RATIO * (double)EVAL_NUM_TRAIN_SAMPLES;
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

		Network* potential_eval_network = new Network(node->obs_histories[0].size());
		uniform_int_distribution<int> match_distribution(0, best_match_obs.size()-1);
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = match_distribution(generator);

			potential_eval_network->activate(best_match_obs[index]);

			double error = best_match_target_vals[index] - potential_eval_network->output->acti_vals[0];

			potential_eval_network->backprop(error);
		}

		double potential_improvement = 0.0;
		for (int h_index = 0; h_index < EVAL_NUM_TEST_SAMPLES; h_index++) {
			bool is_match = is_match_helper(node->obs_histories[EVAL_NUM_TRAIN_SAMPLES + h_index],
											best_potential_obs_index,
											best_potential_rel_obs_index,
											best_potential_split_type,
											best_potential_split_target,
											best_potential_split_range);

			if (is_match) {
				potential_eval_network->activate(node->obs_histories[EVAL_NUM_TRAIN_SAMPLES + h_index]);

				double new_misguess = (node->target_val_histories[EVAL_NUM_TRAIN_SAMPLES + h_index] - potential_eval_network->output->acti_vals[0])
					* (node->target_val_histories[EVAL_NUM_TRAIN_SAMPLES + h_index] - potential_eval_network->output->acti_vals[0]);
				double existing_misguess = (node->target_val_histories[EVAL_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index])
					* (node->target_val_histories[EVAL_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index]);

				potential_improvement += (existing_misguess - new_misguess);
			}
		}

		if (potential_improvement > best_improvement) {
			best_obs_index = best_potential_obs_index;
			best_rel_obs_index = best_potential_rel_obs_index;
			best_split_type = best_potential_split_type;
			best_split_target = best_potential_split_target;
			best_split_range = best_potential_split_range;
			if (best_eval_network != NULL) {
				delete best_eval_network;
			}
			best_eval_network = potential_eval_network;
			best_improvement = potential_improvement;
		} else {
			delete potential_eval_network;
		}
	}

	Network* new_default_network = new Network(node->obs_histories[0].size());
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int random_index = train_distribution(generator);

		new_default_network->activate(node->obs_histories[random_index]);

		double error = node->target_val_histories[random_index] - new_default_network->output->acti_vals[0];

		new_default_network->backprop(error);
	}

	double new_default_improvement = 0.0;
	for (int h_index = 0; h_index < EVAL_NUM_TEST_SAMPLES; h_index++) {
		new_default_network->activate(node->obs_histories[EVAL_NUM_TRAIN_SAMPLES + h_index]);

		double new_misguess = (node->target_val_histories[EVAL_NUM_TRAIN_SAMPLES + h_index] - new_default_network->output->acti_vals[0])
			* (node->target_val_histories[EVAL_NUM_TRAIN_SAMPLES + h_index] - new_default_network->output->acti_vals[0]);
		double existing_misguess = (node->target_val_histories[EVAL_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index])
			* (node->target_val_histories[EVAL_NUM_TRAIN_SAMPLES + h_index] - existing_predicted[h_index]);

		new_default_improvement += (existing_misguess - new_misguess);
	}

	if (new_default_improvement >= best_improvement) {
		delete best_eval_network;

		delete node->network;
		node->network = new_default_network;
	} else {
		delete new_default_network;

		if (best_improvement > 0.0) {
			EvalNode* new_eval_node = new EvalNode();
			new_eval_node->id = this->node_counter;
			this->node_counter++;
			this->nodes[new_eval_node->id] = new_eval_node;

			new_eval_node->network = best_eval_network;

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

			SplitNode* parent = NULL;
			bool is_branch;
			find_parent(node,
						this,
						parent,
						is_branch);

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
