#include "experiment.h"

#include "constants.h"
#include "network.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void Experiment::remeasure_existing_check_activate(
		SolutionWrapper* wrapper) {
	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::remeasure_existing_step(vector<double>& obs,
										 int& action,
										 bool& is_next,
										 SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

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
		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
		history->hit_branch = true;
	}

	delete experiment_state;
	wrapper->experiment_context.back() = NULL;
}

void Experiment::remeasure_existing_backprop(double target_val,
											 SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	if (history->hit_branch) {
		this->num_branch++;
	} else {
		this->num_original++;
		if (this->num_original == BRANCH_RATIO_CHECK_ITER) {
			double branch_ratio = (double)this->num_branch / ((double)this->num_original + (double)this->num_branch);
			if (branch_ratio < BRANCH_MIN_RATIO) {
				this->result = EXPERIMENT_RESULT_FAIL;
				return;
			}
		}
	}

	if (history->hit_branch) {
		this->existing_scores.push_back(target_val);

		this->state_iter++;
		if (this->state_iter >= MEASURE_STEP_NUM_ITERS) {
			this->state = EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
		}
	}
}
