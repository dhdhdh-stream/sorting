#include "logic_helpers.h"

#include <iostream>

#include "constants.h"
#include "eval_node.h"
#include "logic_experiment.h"
#include "network.h"
#include "split_node.h"

using namespace std;

double logic_eval_helper(AbstractLogicNode* node,
						 vector<double>& obs) {
	switch (node->type) {
	case LOGIC_NODE_TYPE_SPLIT:
		{
			SplitNode* split_node = (SplitNode*)node;
			bool is_branch;
			switch (split_node->split_type) {
			case SPLIT_TYPE_GREATER:
				if (obs[split_node->obs_index] > split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_GREATER_EQUAL:
				if (obs[split_node->obs_index] >= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_LESSER:
				if (obs[split_node->obs_index] < split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_LESSER_EQUAL:
				if (obs[split_node->obs_index] <= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHIN:
				if (abs(obs[split_node->obs_index] - split_node->split_target) < split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHIN_EQUAL:
				if (abs(obs[split_node->obs_index] - split_node->split_target) <= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHOUT:
				if (abs(obs[split_node->obs_index] - split_node->split_target) > split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHOUT_EQUAL:
				if (abs(obs[split_node->obs_index] - split_node->split_target) >= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_GREATER:
				if (obs[split_node->obs_index] - obs[split_node->rel_obs_index] > split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_GREATER_EQUAL:
				if (obs[split_node->obs_index] - obs[split_node->rel_obs_index] >= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHIN:
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) < split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHIN_EQUAL:
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) <= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHOUT:
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) > split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHOUT_EQUAL:
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) >= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			}
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

void logic_experiment_helper(AbstractLogicNode* node,
							 vector<double>& obs,
							 double target_val,
							 LogicWrapper* logic_wrapper) {
	if (node->experiment == NULL) {
		node->experiment = new LogicExperiment(node);
	}

	node->experiment->activate(obs,
							   target_val);

	switch (node->type) {
	case LOGIC_NODE_TYPE_SPLIT:
		{
			SplitNode* split_node = (SplitNode*)node;
			bool is_branch;
			switch (split_node->split_type) {
			case SPLIT_TYPE_GREATER:
				if (obs[split_node->obs_index] > split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_GREATER_EQUAL:
				if (obs[split_node->obs_index] >= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_LESSER:
				if (obs[split_node->obs_index] < split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_LESSER_EQUAL:
				if (obs[split_node->obs_index] <= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHIN:
				if (abs(obs[split_node->obs_index] - split_node->split_target) < split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHIN_EQUAL:
				if (abs(obs[split_node->obs_index] - split_node->split_target) <= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHOUT:
				if (abs(obs[split_node->obs_index] - split_node->split_target) > split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_WITHOUT_EQUAL:
				if (abs(obs[split_node->obs_index] - split_node->split_target) >= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_GREATER:
				if (obs[split_node->obs_index] - obs[split_node->rel_obs_index] > split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_GREATER_EQUAL:
				if (obs[split_node->obs_index] - obs[split_node->rel_obs_index] >= split_node->split_target) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHIN:
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) < split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHIN_EQUAL:
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) <= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHOUT:
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) > split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			case SPLIT_TYPE_REL_WITHOUT_EQUAL:
				if (abs((obs[split_node->obs_index] - obs[split_node->rel_obs_index]) - split_node->split_target) >= split_node->split_range) {
					is_branch = true;
				} else {
					is_branch = false;
				}
				break;
			}
			if (is_branch) {
				logic_experiment_helper(split_node->branch_node,
										obs,
										target_val,
										logic_wrapper);
			} else {
				logic_experiment_helper(split_node->original_node,
										obs,
										target_val,
										logic_wrapper);
			}
		}
		break;
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
