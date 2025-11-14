#include "explore_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "explore_instance.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void ExploreExperiment::check_activate(AbstractNode* experiment_node,
									   bool is_branch,
									   SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ExploreExperimentHistory* history = (ExploreExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;

		switch (this->state) {
		case EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_check_activate(wrapper);
			break;
		case EXPLORE_EXPERIMENT_STATE_EXPLORE:
			explore_check_activate(wrapper);
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
	}
}

void ExploreExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	explore_set_action(action,
					   wrapper);
}

void ExploreExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	explore_exit_step(wrapper);
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
	}
}
