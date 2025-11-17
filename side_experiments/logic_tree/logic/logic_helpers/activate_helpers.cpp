#include "logic_helpers.h"

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
			case SPLIT_TYPE_LESSER_EQUAL:
				if (obs[split_node->obs_index] <= split_node->split_target) {
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
			case SPLIT_TYPE_LESSER_EQUAL:
				if (obs[split_node->obs_index] <= split_node->split_target) {
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
