#include "pass_through_experiment.h"

#include "branch_experiment.h"
#include "globals.h"

using namespace std;

void PassThroughExperiment::activate(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 int& exit_depth,
									 AbstractNode*& exit_node,
									 RunHelper& run_helper,
									 AbstractExperimentHistory*& history) {
	if (run_helper.experiment_history == NULL) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			bool select = false;
			bool has_seen = false;
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				if (run_helper.experiments_seen_order[e_index] == this) {
					has_seen = true;
					break;
				}
			}
			if (!has_seen) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					select = true;
				}

				run_helper.experiments_seen_order.push_back(this);
			}
			if (select) {
				PassThroughExperimentOverallHistory* overall_history = new PassThroughExperimentOverallHistory(this);
				run_helper.experiment_history = overall_history;

				switch (this->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
					if (this->sub_state_iter == 0) {
						explore_initial_activate(curr_node,
												 problem,
												 context,
												 exit_depth,
												 exit_node,
												 run_helper);
					} else {
						explore_activate(curr_node,
										 problem,
										 context,
										 exit_depth,
										 exit_node,
										 run_helper);
					}
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW:
					measure_new_activate(curr_node,
										 problem,
										 context,
										 exit_depth,
										 exit_node,
										 run_helper);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW:
				case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW:
					verify_new_activate(curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
					overall_history->branch_experiment_history = new BranchExperimentOverallHistory(this->branch_experiment);

					experiment_activate(curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper,
										history);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW:
				case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_NEW:
					experiment_verify_new_activate(curr_node,
												   problem,
												   context,
												   exit_depth,
												   exit_node,
												   run_helper,
												   history);
					break;
				}
			}
		}
	} else if (run_helper.experiment_history->experiment == this) {
		bool matches_context = true;
		if (this->scope_context.size() > context.size()) {
			matches_context = false;
		} else {
			for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
				if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope
						|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node) {
					matches_context = false;
					break;
				}
			}
		}

		if (matches_context) {
			switch (this->state) {
			case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
				explore_activate(curr_node,
								 problem,
								 context,
								 exit_depth,
								 exit_node,
								 run_helper);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW:
				measure_new_activate(curr_node,
									 problem,
									 context,
									 exit_depth,
									 exit_node,
									 run_helper);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW:
			case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW:
				verify_new_activate(curr_node,
									problem,
									context,
									exit_depth,
									exit_node,
									run_helper);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
				{
					PassThroughExperimentOverallHistory* overall_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
					overall_history->branch_experiment_history = new BranchExperimentOverallHistory(this->branch_experiment);
				}

				experiment_activate(curr_node,
									problem,
									context,
									exit_depth,
									exit_node,
									run_helper,
									history);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW:
			case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_NEW:
				experiment_verify_new_activate(curr_node,
											   problem,
											   context,
											   exit_depth,
											   exit_node,
											   run_helper,
											   history);
				break;
			}
		}
	}
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper,
									 AbstractExperimentHistory* history) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW:
		measure_new_backprop(target_val,
							 run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_EXISTING:
		verify_existing_backprop(target_val,
								 run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_NEW:
	case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND_NEW:
		verify_new_backprop(target_val,
							run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT:
		experiment_backprop(target_val,
							run_helper,
							(PassThroughExperimentOverallHistory*)history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_EXISTING:
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING:
		experiment_verify_existing_backprop(target_val,
											run_helper);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW:
	case PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_NEW:
		experiment_verify_new_backprop(target_val,
									   run_helper);
		break;
	}
}
