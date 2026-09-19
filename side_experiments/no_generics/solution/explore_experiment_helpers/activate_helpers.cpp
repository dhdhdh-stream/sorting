#include "explore_experiment.h"

#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::experiment_check_activate(vector<double>& obs,
												  SolutionWrapper* wrapper) {
	map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it =
		wrapper->experiment_histories.find(this);
	if (it == wrapper->experiment_histories.end()) {
		it = wrapper->experiment_histories.insert({this, new ExploreExperimentHistory(this)}).first;
	}
	ExploreExperimentHistory* explore_experiment_history = (ExploreExperimentHistory*)it->second;

	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_check_activate(obs,
									  explore_experiment_history,
									  wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_check_activate(obs,
							   explore_experiment_history,
							   wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_check_activate(obs,
								 explore_experiment_history,
								 wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_REUSE_MEASURE:
		reuse_measure_check_activate(obs,
									 explore_experiment_history,
									 wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_NEW_STATE_MEASURE:
		new_state_measure_check_activate(obs,
										 explore_experiment_history,
										 wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPLORE_EXPERIMENT_STATE_REUSE_VERIFY:
		reuse_verify_check_activate(obs,
									explore_experiment_history,
									wrapper);
		break;
	#endif /* MDEBUG */
	}
}

void ExploreExperiment::experiment_step(vector<double>& obs,
										int& action,
										bool& is_next,
										bool& fetch_action,
										SolutionWrapper* wrapper) {
	switch (this->state) {
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
	case EXPLORE_EXPERIMENT_STATE_REUSE_MEASURE:
		reuse_measure_step(obs,
						   action,
						   is_next,
						   wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_NEW_STATE_MEASURE:
		new_state_measure_step(obs,
							   action,
							   is_next,
							   wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPLORE_EXPERIMENT_STATE_REUSE_VERIFY:
		reuse_verify_step(obs,
						  action,
						  is_next,
						  wrapper);
		break;
	#endif /* MDEBUG */
	}
}

void ExploreExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	explore_set_action(action,
					   wrapper);
}

void ExploreExperiment::experiment_exit_step(vector<double>& obs,
											 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(obs,
						  wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(obs,
							wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_REUSE_MEASURE:
		reuse_measure_exit_step(obs,
								wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_NEW_STATE_MEASURE:
		new_state_measure_exit_step(obs,
									wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPLORE_EXPERIMENT_STATE_REUSE_VERIFY:
		reuse_verify_exit_step(obs,
							   wrapper);
		break;
	#endif /* MDEBUG */
	}
}

void ExploreExperiment::experiment_step_callback(vector<double>& obs,
												 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_callback(obs,
						 wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_callback(obs,
						   wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_REUSE_MEASURE:
		reuse_measure_callback(obs,
							   wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_NEW_STATE_MEASURE:
		new_state_measure_callback(obs,
								   wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPLORE_EXPERIMENT_STATE_REUSE_VERIFY:
		reuse_verify_callback(obs,
							  wrapper);
		break;
	#endif /* MDEBUG */
	}
}

void ExploreExperiment::backprop(double target_val,
								 AbstractExperimentHistory* history,
								 SolutionWrapper* wrapper) {
	ExploreExperimentHistory* explore_experiment_history = (ExploreExperimentHistory*)history;

	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								explore_experiment_history,
								wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 explore_experiment_history,
						 wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   explore_experiment_history,
						   wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_REUSE_MEASURE:
		reuse_measure_backprop(target_val,
							   explore_experiment_history,
							   wrapper);
		break;
	case EXPLORE_EXPERIMENT_STATE_NEW_STATE_MEASURE:
		new_state_measure_backprop(target_val,
								   explore_experiment_history,
								   wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case EXPLORE_EXPERIMENT_STATE_REUSE_VERIFY:
		reuse_verify_backprop(target_val,
							  explore_experiment_history,
							  wrapper);
		break;
	#endif /* MDEBUG */
	}
}
