#include "explore_experiment.h"

#include "globals.h"
#include "solution_wrapper.h"

using namespace std;



ExploreExperimentHistory::ExploreExperimentHistory(
		ExploreExperiment* experiment,
		SolutionWrapper* wrapper) {
	this->is_on = false;
	if (wrapper->should_explore
			&& wrapper->curr_explore == NULL
			&& experiment->state != EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING
			&& experiment->last_num_explore.size() > 0) {
		double average_num_experiments = (double)experiment->sum_num_explore
			/ (double)experiment->last_num_explore.size();
		uniform_real_distribution<double> distribution(0.0, 1.0);
		double rand_val = distribution(generator);
		if (rand_val <= 1.0 / average_num_experiments) {
			wrapper->curr_explore = experiment;
			this->is_on = true;
		} 
	}

	this->num_instances = 0;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
