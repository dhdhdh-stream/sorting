#include "compare_experiment.h"

#include <iostream>

#include "experiment_run.h"
#include "network.h"
#include "utilities.h"
#include "wrapper.h"

using namespace std;

void CompareExperiment::measure_existing_experiment_activate(
		ExperimentRun* run) {
	bool is_branch;
	this->original_network->activate(run->state);
	this->branch_network->activate(run->state);
	if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	#if defined(MDEBUG) && MDEBUG
	if (run->wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run->wrapper->curr_run_seed = xorshift(run->wrapper->curr_run_seed);
	#endif /* MDEBUG */

	if (is_branch) {
		run->compare_experiment_history->hit_branch = true;
	}
}

void CompareExperiment::measure_existing_backprop(
		double target_val,
		ExperimentRun* run,
		Wrapper* wrapper) {
	if (run->compare_experiment_history->hit_branch) {
		this->sum_scores += target_val;

		this->state_iter++;
		if (this->state_iter >= MEASURE_NUM_ITERS) {
			this->existing_average_score = this->sum_scores / (double)MEASURE_NUM_ITERS;

			this->sum_scores = 0.0;

			this->state = COMPARE_EXPERIMENT_MEASURE_NEW;
			this->state_iter = 0;
		}
	}
}
