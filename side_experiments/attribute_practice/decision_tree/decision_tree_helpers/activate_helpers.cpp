#include "decision_tree.h"

#include "decision_tree_node.h"
#include "network.h"

using namespace std;

double DecisionTree::activate(vector<double>& obs) {
	if (this->root == NULL) {
		return 0.0;
	}

	DecisionTreeNode* curr_node = this->root;
	double previous_val = 0.0;
	while (true) {
		if (curr_node->has_split) {
			previous_val = curr_node->activate(obs,
											   previous_val);

			bool is_branch = is_match_helper(obs,
											 curr_node->obs_index,
											 curr_node->rel_obs_index,
											 curr_node->split_type,
											 curr_node->split_target,
											 curr_node->split_range);
			if (is_branch) {
				curr_node = curr_node->branch_node;
			} else {
				curr_node = curr_node->original_node;
			}
		} else {
			return curr_node->activate(obs,
									   previous_val);
		}
	}

	return 0.0;		// unreachable
}

void DecisionTree::backprop(vector<double>& obs,
							double target_val) {
	if (this->obs_histories.size() < DT_NUM_TOTAL_SAMPLES) {
		this->obs_histories.push_back(obs);
		this->target_val_histories.push_back(target_val);
	} else {
		this->obs_histories[this->history_index] = obs;
		this->target_val_histories[this->history_index] = target_val;

		this->history_index++;
		if (this->history_index >= DT_NUM_TOTAL_SAMPLES) {
			this->history_index = 0;
		}
	}

	if (this->root == NULL) {
		if ((int)this->obs_histories.size() >= DT_NUM_TOTAL_SAMPLES) {
			init_helper();
		}
	} else {
		DecisionTreeNode* curr_node = this->root;
		double previous_val = 0.0;
		while (true) {
			if (curr_node->has_split) {
				previous_val = curr_node->activate(obs,
												   previous_val);

				bool is_branch = is_match_helper(obs,
												 curr_node->obs_index,
												 curr_node->rel_obs_index,
												 curr_node->split_type,
												 curr_node->split_target,
												 curr_node->split_range);
				if (is_branch) {
					curr_node = curr_node->branch_node;
				} else {
					curr_node = curr_node->original_node;
				}
			} else {
				curr_node->obs_histories.push_back(obs);
				curr_node->previous_val_histories.push_back(previous_val);
				curr_node->target_val_histories.push_back(target_val);
				if (curr_node->obs_histories.size() >= DT_NUM_TOTAL_SAMPLES) {
					update_helper(curr_node);
				}

				break;
			}
		}
	}
}

void DecisionTree::measure_helper() {
	double sum_misguess = 0.0;
	for (int h_index = 0; h_index < (int)this->obs_histories.size(); h_index++) {
		double predicted_val = activate(this->obs_histories[h_index]);
		sum_misguess += (this->target_val_histories[h_index] - predicted_val) * (this->target_val_histories[h_index] - predicted_val);
	}

	this->improvement_history.push_back(sum_misguess / this->obs_histories.size());
}
