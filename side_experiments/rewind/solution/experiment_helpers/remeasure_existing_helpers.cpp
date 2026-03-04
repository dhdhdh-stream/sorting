#include "experiment.h"

#include "network.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int REMEASURE_EXISTING_NUM_DATAPOINTS = 20;
#else
const int REMEASURE_EXISTING_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

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
		this->num_branch++;

		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
		history->hit_branch = true;
	} else {
		this->num_original++;
	}

	delete experiment_state;
	wrapper->experiment_context.back() = NULL;
}

void Experiment::remeasure_existing_backprop(double target_val,
											 SolutionWrapper* wrapper) {
	if (this->num_original > 20000) {
		double branch_ratio = (double)this->num_branch / ((double)this->num_original + (double)this->num_branch);
		if (branch_ratio < 0.05) {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}

	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (history->hit_branch) {
		this->true_scores.push_back(target_val);

		if ((int)this->true_scores.size() >= REMEASURE_EXISTING_NUM_DATAPOINTS) {
			double sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->true_scores.size(); h_index++) {
				sum_vals += this->true_scores[h_index];
			}
			this->existing_true = sum_vals / (double)this->true_scores.size();

			this->true_scores.clear();

			this->num_original = 0;
			this->num_branch = 0;

			this->total_count = 0;
			this->total_sum_scores = 0.0;

			this->state = EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
		}
	}
}
