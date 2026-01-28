#include "logic_helpers.h"

#include <iostream>

#include "constants.h"
#include "eval_node.h"
#include "logic_tree.h"
#include "logic_wrapper.h"
#include "network.h"
#include "split_node.h"

using namespace std;

double logic_eval_helper(AbstractLogicNode* node,
						 vector<double>& obs) {
	switch (node->type) {
	case LOGIC_NODE_TYPE_SPLIT:
		{
			SplitNode* split_node = (SplitNode*)node;
			bool is_branch = is_match_helper(obs,
											 split_node->obs_index,
											 split_node->rel_obs_index,
											 split_node->split_type,
											 split_node->split_target,
											 split_node->split_range);
			if (is_branch) {
				return logic_eval_helper(split_node->branch_node,
										 obs);
			} else {
				return logic_eval_helper(split_node->original_node,
										 obs);
			}
		}
		break;
	case LOGIC_NODE_TYPE_EVAL:
		{
			EvalNode* eval_node = (EvalNode*)node;
			eval_node->network->activate(obs);
			return eval_node->network->output->acti_vals[0];
		}
		break;
	}

	return 0.0;
}

void logic_experiment_helper(vector<double>& obs,
							 double target_val,
							 LogicWrapper* logic_wrapper,
							 AbstractProblem* problem) {
	vector<AbstractLogicNode*> hit_nodes;

	AbstractLogicNode* curr_node = logic_wrapper->logic_tree->root;
	double existing_predicted;
	while (true) {
		hit_nodes.push_back(curr_node);

		if (curr_node->type == LOGIC_NODE_TYPE_SPLIT) {
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
			existing_predicted = eval_node->network->output->acti_vals[0];

			break;
		}
	}

	bool is_update = false;
	for (int n_index = 0; n_index < (int)hit_nodes.size(); n_index++) {
		hit_nodes[n_index]->obs_histories.push_back(obs);
		hit_nodes[n_index]->target_val_histories.push_back(target_val);
		hit_nodes[n_index]->existing_predicted_histories.push_back(existing_predicted);

		if (hit_nodes[n_index]->obs_histories.size() >= NUM_TOTAL_SAMPLES) {
			update(hit_nodes[n_index],
				   logic_wrapper);

			is_update = true;
		}
	}

	if (is_update) {
		logic_wrapper->logic_tree->clean();

		double curr_misguess = measure_helper(problem,
											  logic_wrapper);
		logic_wrapper->logic_tree->improvement_history.push_back(curr_misguess);

		cout << "curr_misguess: " << curr_misguess << endl;
	}
}

double view_logic_eval_helper(AbstractLogicNode* node,
							  vector<double>& obs) {
	switch (node->type) {
	case LOGIC_NODE_TYPE_SPLIT:
		{
			SplitNode* split_node = (SplitNode*)node;
			bool is_branch;
			switch (split_node->split_type) {
			case SPLIT_TYPE_GREATER:
				cout << split_node->obs_index << " greater than " << split_node->split_target << endl;
				if (obs[split_node->obs_index] > split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_GREATER_EQUAL:
				cout << split_node->obs_index << " greater equal than " << split_node->split_target << endl;
				if (obs[split_node->obs_index] >= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_LESSER:
				cout << split_node->obs_index << " lesser than " << split_node->split_target << endl;
				if (obs[split_node->obs_index] < split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_LESSER_EQUAL:
				cout << split_node->obs_index << " lesser equal than " << split_node->split_target << endl;
				if (obs[split_node->obs_index] <= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHIN:
				cout << split_node->obs_index << " within " << split_node->split_range << " of " << split_node->split_target << endl;
				if (abs(obs[split_node->obs_index] - split_node->split_target) < split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHIN_EQUAL:
				cout << split_node->obs_index << " within equal " << split_node->split_range << " of " << split_node->split_target << endl;
				if (abs(obs[split_node->obs_index] - split_node->split_target) <= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHOUT:
				cout << split_node->obs_index << " without " << split_node->split_range << " of " << split_node->split_target << endl;
				if (abs(obs[split_node->obs_index] - split_node->split_target) > split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHOUT_EQUAL:
				cout << split_node->obs_index << " without equal " << split_node->split_range << " of " << split_node->split_target << endl;
				if (abs(obs[split_node->obs_index] - split_node->split_target) >= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_GREATER:
				cout << split_node->obs_index << " greater than " << split_node->rel_obs_index << " by " << split_node->split_target << endl;
				if (obs[split_node->obs_index] - obs[split_node->rel_obs_index] > split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_GREATER_EQUAL:
				cout << split_node->obs_index << " greater equal than " << split_node->rel_obs_index << " by " << split_node->split_target << endl;
				if (obs[split_node->obs_index] - obs[split_node->rel_obs_index] >= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHIN:
				cout << split_node->obs_index << " diff " << split_node->rel_obs_index << " within " << split_node->split_range << " of " << split_node->split_target << endl;
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) < split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHIN_EQUAL:
				cout << split_node->obs_index << " diff " << split_node->rel_obs_index << " within equal " << split_node->split_range << " of " << split_node->split_target << endl;
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) <= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHOUT:
				cout << split_node->obs_index << " diff " << split_node->rel_obs_index << " without " << split_node->split_range << " of " << split_node->split_target << endl;
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) > split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHOUT_EQUAL:
				cout << split_node->obs_index << " diff " << split_node->rel_obs_index << " without equal " << split_node->split_range << " of " << split_node->split_target << endl;
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) >= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			}
			if (is_branch) {
				return view_logic_eval_helper(split_node->branch_node,
											  obs);
			} else {
				return view_logic_eval_helper(split_node->original_node,
											  obs);
			}
		}
		break;
	case LOGIC_NODE_TYPE_EVAL:
		{
			EvalNode* eval_node = (EvalNode*)node;
			eval_node->network->activate(obs);
			return eval_node->network->output->acti_vals[0];
		}
		break;
	}

	return 0.0;
}
