#include "eval_pass_through_experiment.h"

using namespace std;

bool EvalPassThroughExperiment::activate(AbstractNode* experiment_node,
										 bool is_branch,
										 AbstractNode*& curr_node,
										 Problem* problem,
										 vector<ContextLayer>& context,
										 RunHelper& run_helper) {
	if (is_branch != this->is_branch) {
		return false;
	} else {
		switch (this->state) {
		case EVAL_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 run_helper);
			break;
		case EVAL_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node,
							   problem,
							   run_helper);
			break;
		case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 run_helper);
			break;
		}

		return true;
	}
}

void EvalPassThroughExperiment::backprop(EvalHistory* eval_history,
										 Problem* problem,
										 vector<ContextLayer>& context,
										 RunHelper& run_helper) {
	switch (this->state) {
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_backprop(problem,
										context,
										run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(eval_history,
								  problem,
								  context,
								  run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(problem,
						 context,
						 run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(eval_history,
						   problem,
						   context,
						   run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(eval_history,
						 problem,
						 context,
						 run_helper);
		break;
	}
}
