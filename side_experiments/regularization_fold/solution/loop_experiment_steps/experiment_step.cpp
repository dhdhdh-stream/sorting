#include "loop_experiment.h"

using namespace std;

void LoopExperiment::experiment_activate(vector<double>& flat_vals,
										 vector<ForwardContextLayer>& context,
										 RunHelper& run_helper,
										 BranchExperimentHistory* history) {
	run_helper.explore_phase = EXPLORE_PHASE_EXPERIMENT;

	history->existing_predicted_score = predicted_score;

	run_helper.experiment = this;
	if (rand()%5 == 0) {
		run_helper.can_zero = true;
	} else {
		run_helper.can_zero = false;
	}
	run_helper.new_state_vals = vector<double>(NUM_NEW_STATES, 0.0);

	int loop_iters = rand()%7;

	for ()

}
