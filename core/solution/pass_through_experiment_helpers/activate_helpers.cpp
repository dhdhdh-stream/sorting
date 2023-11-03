#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::activate(int& curr_node_id,
									 Problem& problem,
									 vector<ContextLayer>& context,
									 int& exit_depth,
									 int& exit_node_id,
									 RunHelper& run_helper,
									 AbstractExperimentHistory*& history) {
	bool matches_context = true;
	if (this->scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
			if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope_id
					|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node_id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (run_helper.selected_experiment == this) {
			switch (this->state) {
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
				measure_existing_score_activate(context);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
				explore_activate(curr_node_id,
								 problem,
								 context,
								 exit_depth,
								 exit_node_id,
								 run_helper);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
				measure_new_score_activate(curr_node_id,
										   problem,
										   context,
										   exit_depth,
										   exit_node_id,
										   run_helper,
										   history);
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_TRAIN:
				train_activate(curr_node_id,
							   problem,
							   context,
							   exit_depth,
							   exit_node_id,
							   run_helper,
							   history);
				break;
			}
		} else if (run_helper.selected_experiment == NULL) {
			bool select = false;
			map<AbstractExperiment*, int>::iterator it = run_helper.experiments_seen.find(this);
			if (it == run_helper.experiments_seen_counts.end()) {
				double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
				uniform_real_distribution<double> distribution(0.0, 1.0);
				if (distribution(generator) < selected_probability) {
					select = true;
				}
			}
			if (select) {
				hook(context);

				run_helper.select_experiment = this;
				run_helper.experiment_history = new PassThroughExperimentOverallHistory(this);

				switch (this->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
					measure_existing_score_activate(context);
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
					if (this->sub_state_iter == 0) {
						explore_initial_activate(curr_node_id,
												 problem,
												 context,
												 exit_depth,
												 exit_node_id,
												 run_helper);
					} else {
						explore_activate(curr_node_id,
										 problem,
										 context,
										 exit_depth,
										 exit_node_id,
										 run_helper);
					}
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
					measure_new_score_activate(curr_node_id,
											   problem,
											   context,
											   exit_depth,
											   exit_node_id,
											   run_helper,
											   history);
					break;
				}
			} else {
				if (it == run_helper.experiments_seen_counts.end()) {
					run_helper.experiments_seen_order.push_back(this);
					run_helper.experiments_seen_counts[this] = 1;
				}
				// else don't need to increment experiments_seen_counts for PassThroughExperiment
			}
		}
	}
}

void PassThroughExperiment::hook_helper() {

}

void PassThroughExperiment::hook() {

}

void PassThroughExperiment::unhook() {

}

void PassThroughExperiment::parent_scope_end_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* parent_scope_history) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_parent_scope_end_activate(
			context,
			run_helper,
			parent_scope_history);
		break;
	}
}

void PassThroughExperiment::backprop(double target_val,
									 PassThroughExperimentOverallHistory* history) {
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE:
		measure_existing_score_backprop(target_val,
										history);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val);
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_NEW_SCORE:
		measure_new_score_backprop(target_val,
								   history);
		break;
	}
}
