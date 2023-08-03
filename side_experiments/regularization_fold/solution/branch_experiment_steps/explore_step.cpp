#include "branch_experiment.h"

using namespace std;

void BranchExperiment::explore_activate(vector<double>& flat_vals,
										vector<ForwardContextLayer>& context,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPLORE;

	// no need to append to context yet

	vector<vector<double>> sequence_input_vals(this->num_steps);
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			double obs = flat_vals.begin();
			flat_vals.erase(flat_vals.begin());
		} else {
			sequence_input_vals[a_index] = vector<double>(this->sequences[a_index]->input_types.size(), 0.0);
			this->sequences[a_index]->activate_pull(sequence_input_vals[a_index],
													context,
													sequence_input_vals,
													history,
													run_helper);

			SequenceHistory* sequence_history = new SequenceHistory(this->sequences[a_index]);
			this->sequences[a_index]->activate(sequence_input_vals[a_index],
											   flat_vals,
											   run_helper,
											   sequence_history);
			delete sequence_history;
		}
	}

	for (int a_index = this->num_steps-1; a_index >= 0; a_index--) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE) {
			this->sequences[a_index]->activate_reset(sequence_input_vals[a_index],
													 context,
													 sequence_input_vals);
		}
	}
}
