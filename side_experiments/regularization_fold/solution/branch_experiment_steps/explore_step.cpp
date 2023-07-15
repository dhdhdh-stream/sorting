#include "branch_experiment.h"

using namespace std;

void BranchExperiment::explore_activate(vector<double>& flat_vals,
										vector<ForwardContextLayer>& context,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPLORE;

	// no need to append to context yet

	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == EXPLORE_STEP_TYPE_ACTION) {
			double obs = flat_vals.begin();
			flat_vals.erase(flat_vals.begin());
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->sequences[a_index]);
			this->sequences[a_index]->activate(flat_vals,
											   context,
											   history,
											   run_helper,
											   sequence_history);
			delete sequence_history;
		}
	}
}
