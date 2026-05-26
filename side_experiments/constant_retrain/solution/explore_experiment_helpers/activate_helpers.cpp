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
		ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->explore_experiment_history;
		history->is_hit = true;

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
	}
}

void ExploreExperiment::backprop(double target_val,
								 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   wrapper);
		break;
	}
}
