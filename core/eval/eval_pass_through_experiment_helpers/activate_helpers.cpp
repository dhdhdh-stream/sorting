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
		case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
		case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
			verify_activate(curr_node,
							problem,
							run_helper);
			break;
		}

		return true;
	}
}

void EvalPassThroughExperiment::back_activate(Problem* problem,
											  ScopeHistory*& subscope_history,
											  RunHelper& run_helper) {
	switch (this->state) {
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_back_activate(subscope_history,
									   run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_back_activate(subscope_history,
								run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
		measure_back_activate(subscope_history,
							  run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_back_activate(subscope_history,
									  run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
		verify_back_activate(subscope_history,
							 run_helper);
		break;
	}
}

void EvalPassThroughExperiment::backprop(double target_val,
										 RunHelper& run_helper) {
	switch (this->state) {
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_backprop(target_val,
										run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
	case EVAL_PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
		verify_backprop(target_val,
						run_helper);
		break;
	}
}
