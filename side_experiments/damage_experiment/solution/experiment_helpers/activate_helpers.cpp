#include "experiment.h"

#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Experiment::check_activate(AbstractNode* experiment_node,
								bool is_branch,
								SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;

		switch (this->state) {
		case EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_check_activate(wrapper);
			break;
		case EXPERIMENT_STATE_EXPLORE:
			explore_check_activate(wrapper);
			break;
		case EXPERIMENT_STATE_TRAIN_NEW:
			train_new_check_activate(wrapper);
			break;
		case EXPERIMENT_STATE_REMEASURE_EXISTING:
			remeasure_existing_check_activate(wrapper);
			break;
		case EXPERIMENT_STATE_MEASURE:
			measure_check_activate(wrapper);
			break;
		}
	}
}

void Experiment::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_step(obs,
							wrapper);
		break;
	case EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper);
		break;
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper);
		break;
	case EXPERIMENT_STATE_REMEASURE_EXISTING:
		remeasure_existing_step(obs,
								action,
								is_next,
								wrapper);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper);
		break;
	}
}

void Experiment::set_action(int action,
							SolutionWrapper* wrapper) {
	explore_set_action(action,
					   wrapper);
}

void Experiment::experiment_exit_step(SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper);
		break;
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_exit_step(wrapper);
		break;
	}
}

void Experiment::backprop(double target_val,
						  SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								wrapper);
		break;
	case EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 wrapper);
		break;
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   wrapper);
		break;
	case EXPERIMENT_STATE_REMEASURE_EXISTING:
		remeasure_existing_backprop(target_val,
									wrapper);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 wrapper);
		break;
	}
}
