#include "loop_experiment.h"

using namespace std;

void LoopExperiment::explore_activate(vector<double>& flat_vals,
									  vector<ForwardContextLayer>& context,
									  RunHelper& run_helper,
									  BranchExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPLORE;

	int rand_iters = 3 + rand()%3;

	for (int i_index = 0; i_index < rand_iters; i_index++) {
		SequenceHistory* sequence_history = new SequenceHistory(this->sequence);
		this->sequence->activate(flat_vals,
								 context,
								 NULL,
								 run_helper,
								 sequence_history);
		delete sequence_history;
	}
}
