#include "signal_experiment.h"

#include "constants.h"
#include "globals.h"

using namespace std;

void SignalExperiment::pre_activate(Problem* problem,
									SignalExperimentHistory* history) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_FIND_SAFE:
		find_safe_pre_activate(problem);
		break;
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_pre_activate(problem,
							 history);
		break;
	}
}

void SignalExperiment::post_activate(Problem* problem,
									 SignalExperimentHistory* history) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_FIND_SAFE:
		find_safe_post_activate(problem);
		break;
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_post_activate(problem,
							  history);
		break;
	}
}

void SignalExperiment::backprop(double target_val,
								SignalExperimentHistory* history) {
	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_MEASURE_EXISTING:
		this->existing_scores.push_back(target_val);

		if ((int)this->existing_scores.size() >= MEASURE_ITERS) {
			geometric_distribution<int> num_actions_distribution(0.2);
			uniform_int_distribution<int> action_distribution(0, 2);
			int num_pre = num_actions_distribution(generator);
			for (int a_index = 0; a_index < num_pre; a_index++) {
				this->pre_actions.push_back(action_distribution(generator));
			}
			int num_post = 5 + num_actions_distribution(generator);
			for (int a_index = 0; a_index < num_post; a_index++) {
				this->post_actions.push_back(action_distribution(generator));
			}

			this->state = SIGNAL_EXPERIMENT_STATE_FIND_SAFE;
		}

		break;
	case SIGNAL_EXPERIMENT_STATE_FIND_SAFE:
		find_safe_backprop(target_val);
		break;
	case SIGNAL_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 history);
		break;
	}
}
