#include "outer_experiment.h"

#include <iostream>

#include "constants.h"
#include "network.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void OuterExperiment::remeasure_existing_check_activate(
		SolutionWrapper* wrapper) {
	OuterExperimentState* new_experiment_state = new OuterExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void OuterExperiment::remeasure_existing_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		SolutionWrapper* wrapper) {
	OuterExperimentState* experiment_state = (OuterExperimentState*)wrapper->experiment_context.back();

	bool is_branch = true;
	for (int n_index = 0; n_index < (int)this->new_networks.size(); n_index++) {
		this->new_networks[n_index]->activate(obs);
		if (this->new_networks[n_index]->output->acti_vals[0] < 0.0) {
			is_branch = false;
			break;
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
	#endif /* MDEBUG */

	if (is_branch) {
		OuterExperimentHistory* history = (OuterExperimentHistory*)wrapper->outer_experiment_history;
		history->hit_branch = true;
	}

	delete experiment_state;
	wrapper->experiment_context.back() = NULL;
}

void OuterExperiment::remeasure_existing_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	OuterExperimentHistory* history = (OuterExperimentHistory*)wrapper->outer_experiment_history;

	if (history->hit_branch) {
		this->existing_scores.push_back(target_val);

		this->state_iter++;
		if (this->state_iter >= OUTER_MEASURE_STEP_NUM_ITERS) {
			this->state = OUTER_EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
		}
	}
}
