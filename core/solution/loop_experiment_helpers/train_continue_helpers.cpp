#include "loop_experiment.h"

using namespace std;

void LoopExperiment::train_continue_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	bool is_target = false;
	LoopExperimentOverallHistory* overall_history = (LoopExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->instance_count++;
	if (!overall_history->has_target) {
		double target_probability;
		if (overall_history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - overall_history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		overall_history->has_target = true;

		train_continue_target_activate(problem,
									   context,
									   run_helper,
									   history);
	} else {
		if (this->state != LOOP_EXPERIMENT_STATE_TRAIN_PRE) {
			train_continue_non_target_activate(problem,
											   context,
											   run_helper,
											   history);
		}
	}
}

void LoopExperiment::train_continue_target_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	
}

void LoopExperiment::train_continue_non_target_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	
}
