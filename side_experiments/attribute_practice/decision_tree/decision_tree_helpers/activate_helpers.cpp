#include "decision_tree.h"

#include "eval_node.h"
#include "network.h"
#include "split_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int DECISION_TREE_MEASURE_NUM_SAMPLES = 40;
#else
const int DECISION_TREE_MEASURE_NUM_SAMPLES = 4000;
#endif /* MDEBUG */

double DecisionTree::activate(vector<double>& obs) {
	if (this->root == NULL) {
		return 0.0;
	}

	AbstractDecisionTreeNode* curr_node = this->root;
	while (true) {
		if (curr_node->type == DECISION_TREE_NODE_TYPE_SPLIT) {
			SplitNode* split_node = (SplitNode*)curr_node;
			bool is_branch = is_match_helper(obs,
											 split_node->obs_index,
											 split_node->rel_obs_index,
											 split_node->split_type,
											 split_node->split_target,
											 split_node->split_range);
			if (is_branch) {
				curr_node = split_node->branch_node;
			} else {
				curr_node = split_node->original_node;
			}
		} else {
			EvalNode* eval_node = (EvalNode*)curr_node;
			eval_node->network->activate(obs);
			return eval_node->network->output->acti_vals[0];
		}
	}

	return 0.0;		// unreachable
}

void DecisionTree::backprop(vector<double>& obs,
							double target_val) {
	if (this->obs_histories.size() < DECISION_TREE_MEASURE_NUM_SAMPLES) {
		this->obs_histories.push_back(obs);
		this->target_val_histories.push_back(target_val);
	} else {
		this->obs_histories[this->history_index] = obs;
		this->target_val_histories[this->history_index] = target_val;

		this->history_index++;
		if (this->history_index >= DECISION_TREE_MEASURE_NUM_SAMPLES) {
			this->history_index = 0;
		}
	}

	if (this->root == NULL) {
		if ((int)this->obs_histories.size() >= DECISION_TREE_MEASURE_NUM_SAMPLES) {
			init_helper();
		}
	} else {
		AbstractDecisionTreeNode* curr_node = this->root;
		while (true) {
			if (curr_node->type == DECISION_TREE_NODE_TYPE_SPLIT) {
				SplitNode* split_node = (SplitNode*)curr_node;
				bool is_branch = is_match_helper(obs,
												 split_node->obs_index,
												 split_node->rel_obs_index,
												 split_node->split_type,
												 split_node->split_target,
												 split_node->split_range);
				if (is_branch) {
					curr_node = split_node->branch_node;
				} else {
					curr_node = split_node->original_node;
				}
			} else {
				EvalNode* eval_node = (EvalNode*)curr_node;

				eval_node->obs_histories.push_back(obs);
				eval_node->target_val_histories.push_back(target_val);
				if (eval_node->obs_histories.size() >= EVAL_NUM_TOTAL_SAMPLES) {
					update_helper(eval_node);
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
