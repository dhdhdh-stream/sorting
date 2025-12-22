#include "chase_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

void ChaseExperiment::check_activate(AbstractNode* experiment_node,
									 bool is_branch,
									 SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;

		switch (this->state) {
		case CHASE_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_check_activate(wrapper);
			break;
		case CHASE_EXPERIMENT_STATE_EXPLORE:
			explore_check_activate(wrapper);
			break;
		case CHASE_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_check_activate(wrapper);
			break;
		case CHASE_EXPERIMENT_STATE_MEASURE:
			measure_check_activate(wrapper);
			break;
		#if defined(MDEBUG) && MDEBUG
		case CHASE_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_check_activate(wrapper);
			break;
		#endif /* MDEBUG */
		}
	}
}

void ChaseExperiment::experiment_step(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  bool& fetch_action,
									  SolutionWrapper* wrapper) {
	switch (this->state) {
	case CHASE_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_step(obs,
							wrapper);
		break;
	case CHASE_EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper);
		break;
	case CHASE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper);
		break;
	case CHASE_EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case CHASE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_step(obs,
							action,
							is_next,
							wrapper);
		break;
	#endif /* MDEBUG */
	}
}

void ChaseExperiment::set_action(int action,
								 SolutionWrapper* wrapper) {
	explore_set_action(action,
					   wrapper);
}

void ChaseExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	switch (this->state) {
	case CHASE_EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper);
		break;
	case CHASE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper);
		break;
	case CHASE_EXPERIMENT_STATE_MEASURE:
		measure_exit_step(wrapper);
		break;
	#if defined(MDEBUG) && MDEBUG
	case CHASE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_exit_step(wrapper);
		break;
	#endif /* MDEBUG */
	}
}

void ChaseExperiment::backprop(double target_val,
							   SolutionWrapper* wrapper) {
	switch (this->state) {
	case CHASE_EXPERIMENT_STATE_TRAIN_EXISTING:
		train_existing_backprop(target_val,
								wrapper);

		delete wrapper->scope_histories[0];

		break;
	case CHASE_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 wrapper);

		delete wrapper->scope_histories[0];

		break;
	case CHASE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   wrapper);

		delete wrapper->scope_histories[0];

		break;
	case CHASE_EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 wrapper);

		if (this->new_scope_histories.size() < MEASURE_ITERS) {
			this->new_scope_histories.push_back(wrapper->scope_histories[0]);
			this->new_target_val_histories.push_back(target_val);
		} else {
			uniform_int_distribution<int> distribution(0, this->new_scope_histories.size()-1);
			int random_index = distribution(generator);
			delete this->new_scope_histories[random_index];
			this->new_scope_histories[random_index] = wrapper->scope_histories[0];
			this->new_target_val_histories[random_index] = target_val;
		}

		break;
	#if defined(MDEBUG) && MDEBUG
	case CHASE_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop(wrapper);

		delete wrapper->scope_histories[0];

		break;
	#endif /* MDEBUG */
	}
}
