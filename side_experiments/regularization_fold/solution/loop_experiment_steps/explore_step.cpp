#include "loop_experiment.h"

using namespace std;

void LoopExperiment::explore_activate(vector<double>& flat_vals,
									  vector<ForwardContextLayer>& context,
									  RunHelper& run_helper,
									  LoopExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPLORE;

	int rand_iters = 3 + rand()%3;

	vector<double> input_vals(this->sequence->input_types.size(), 0.0);
	vector<vector<double>> empty_previous_vals;
	this->sequence->activate_pull(input_vals,
								  context,
								  empty_previous_vals,
								  NULL,
								  run_helper);

	for (int i_index = 0; i_index < rand_iters; i_index++) {
		SequenceHistory* sequence_history = new SequenceHistory(this->sequence);
		this->sequence->activate(input_vals,
								 flat_vals,
								 run_helper,
								 sequence_history);
		delete sequence_history;
	}

	this->sequence->activate_reset(input_vals,
								   context,
								   empty_previous_vals);
}
