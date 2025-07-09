#include "branch_compare_experiment.h"

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

void BranchCompareExperiment::check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		BranchCompareExperimentHistory* history = (BranchCompareExperimentHistory*)wrapper->experiment_history;
		history->is_hit = true;

		switch (this->state) {
		case BRANCH_COMPARE_EXPERIMENT_STATE_EXPLORE:
			explore_check_activate(wrapper,
								   history);
			break;
		case BRANCH_COMPARE_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_check_activate(wrapper,
									 history);
			break;
		case BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_TRUE:
			measure_true_check_activate(wrapper);
			break;
		case BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_SIGNAL:
			measure_signal_check_activate(wrapper);
			break;
		}
	}
}

void BranchCompareExperiment::experiment_step(
		vector<double>& obs,
		int& action,
		bool& is_next,
		bool& fetch_action,
		SolutionWrapper* wrapper) {
	BranchCompareExperimentState* experiment_state = (BranchCompareExperimentState*)wrapper->experiment_context.back();
	switch (this->state) {
	case BRANCH_COMPARE_EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper,
					 experiment_state);
		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper,
					   experiment_state);
		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_TRUE:
		measure_true_step(obs,
						  action,
						  is_next,
						  wrapper,
						  experiment_state);
		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_SIGNAL:
		measure_signal_step(obs,
							action,
							is_next,
							wrapper,
							experiment_state);
		break;
	}
}

void BranchCompareExperiment::set_action(
		int action,
		SolutionWrapper* wrapper) {
	BranchCompareExperimentState* experiment_state = (BranchCompareExperimentState*)wrapper->experiment_context.back();
	explore_set_action(action,
					   experiment_state);
}

void BranchCompareExperiment::experiment_exit_step(
		SolutionWrapper* wrapper) {
	BranchCompareExperimentState* experiment_state = (BranchCompareExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	switch (this->state) {
	case BRANCH_COMPARE_EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper,
						  experiment_state);
		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper,
							experiment_state);
		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_TRUE:
		measure_true_exit_step(wrapper,
							   experiment_state);
		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_SIGNAL:
		measure_signal_exit_step(wrapper,
								 experiment_state);
		break;
	}
}

void BranchCompareExperiment::back_activate(
		SolutionWrapper* wrapper) {
	switch (this->state) {
	case BRANCH_COMPARE_EXPERIMENT_STATE_EXPLORE:
		explore_back_activate(wrapper);
		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_back_activate(wrapper);
		break;
	}
}

void BranchCompareExperiment::backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	BranchCompareExperimentHistory* history = (BranchCompareExperimentHistory*)wrapper->experiment_history;
	switch (this->state) {
	case BRANCH_COMPARE_EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 history);

		if (wrapper->solution->explore_scope_histories.size() < NUM_EXPLORE_SAVE) {
			wrapper->solution->explore_scope_histories.push_back(wrapper->scope_histories[0]);
			wrapper->solution->explore_target_val_histories.push_back(target_val);
		} else {
			uniform_int_distribution<int> distribution(0, wrapper->solution->explore_scope_histories.size()-1);
			int random_index = distribution(generator);
			delete wrapper->solution->explore_scope_histories[random_index];
			wrapper->solution->explore_scope_histories[random_index] = wrapper->scope_histories[0];
			wrapper->solution->explore_target_val_histories[random_index] = target_val;
		}

		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   history);

		delete wrapper->scope_histories[0];

		break;
	case BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_TRUE:
		measure_true_backprop(target_val,
							  history);

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
	case BRANCH_COMPARE_EXPERIMENT_STATE_MEASURE_SIGNAL:
		measure_signal_backprop(target_val,
								history);

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
	}
}
