#include "explore_experiment.h"

#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::experiment_check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it =
			wrapper->explore_experiment_histories.find(this);
		if (it == wrapper->explore_experiment_histories.end()) {
			it = wrapper->explore_experiment_histories.insert({this, new ExploreExperimentHistory(this)}).first;
		}
		it->second->num_instances++;

		switch (this->state) {
		case EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_check_activate(wrapper);
			break;
		case EXPLORE_EXPERIMENT_STATE_EXPLORE:
			explore_check_activate(wrapper);
			break;
		case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_check_activate(wrapper);
			break;
		case EXPLORE_EXPERIMENT_STATE_MEASURE:
			measure_check_activate(wrapper);
			break;
		}
	}
}

void ExploreExperiment::experiment_step(vector<double>& obs,
										int& action,
										bool& is_next,
										bool& fetch_action,
										SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_step(obs,
							wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper);
		break;
	}
}

void ExploreExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	explore_set_action(action,
					   wrapper);
}

void ExploreExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_MEASURE:
		measure_exit_step(wrapper);
		break;
	}
}

void ExploreExperiment::backprop(double target_val,
								 ExploreExperimentHistory* history,
								 SolutionWrapper* wrapper) {
	this->average_instances_per_run = 0.99*this->average_instances_per_run + 0.01*history->num_instances;

	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								history,
								wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 history,
						 wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   history,
						   wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 history,
						 wrapper);
		break;
	}
}
