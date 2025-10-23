#include "branch_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void BranchExperiment::result_check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;
	}
}

void BranchExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;

		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			explore_check_activate(wrapper,
								   history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_check_activate(wrapper,
									 history);
			break;
		case BRANCH_EXPERIMENT_STATE_MEASURE:
			measure_check_activate(wrapper,
								   history);
			break;
		#if defined(MDEBUG) && MDEBUG
		case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
			capture_verify_check_activate(wrapper);
			break;
		#endif /* MDEBUG */
		}
	}
}

void BranchExperiment::experiment_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   bool& fetch_action,
									   SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper,
					 experiment_state);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper,
					   experiment_state);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_step(obs,
					 action,
					 is_next,
					 wrapper,
					 experiment_state);
		break;
	#if defined(MDEBUG) && MDEBUG
	case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_step(obs,
							action,
							is_next,
							wrapper,
							experiment_state);
		break;
	#endif /* MDEBUG */
	}
}

void BranchExperiment::set_action(int action,
								  SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context.back();
	explore_set_action(action,
					   experiment_state);
}

void BranchExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	BranchExperimentState* experiment_state = (BranchExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper,
						  experiment_state);
		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper,
							experiment_state);
		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
		measure_exit_step(wrapper,
						  experiment_state);
		break;
	#if defined(MDEBUG) && MDEBUG
	case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_exit_step(wrapper,
								 experiment_state);
		break;
	#endif /* MDEBUG */
	}
}

void BranchExperiment::backprop(double target_val,
								SolutionWrapper* wrapper) {
	switch (this->state) {
	case BRANCH_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 wrapper);

		delete wrapper->scope_histories[0];

		break;
	case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   wrapper);

		delete wrapper->scope_histories[0];

		break;
	case BRANCH_EXPERIMENT_STATE_MEASURE:
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
	case BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop(wrapper);

		delete wrapper->scope_histories[0];

		break;
	#endif /* MDEBUG */
	}
}
