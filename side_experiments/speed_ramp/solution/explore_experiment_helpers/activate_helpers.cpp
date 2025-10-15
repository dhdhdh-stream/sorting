#include "explore_experiment.h"

#include <iostream>

#include "constants.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::check_activate(AbstractNode* experiment_node,
									   bool is_branch,
									   SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		ExploreExperimentHistory* history;
		map<ExploreExperiment*, ExploreExperimentHistory*>::iterator it =
			wrapper->explore_histories.find(this);
		if (it == wrapper->explore_histories.end()) {
			history = new ExploreExperimentHistory(this,
												   wrapper);
			wrapper->explore_histories[this] = history;

			wrapper->explore_order_seen.push_back(this);
		} else {
			history = it->second;
		}

		history->num_instances++;

		switch (this->state) {
		case EXPLORE_EXPERIMENT_STATE_EXPLORE:
			explore_check_activate(wrapper,
								   history);
			break;
		case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_check_activate(wrapper,
									 history);
			break;
		}
	}
}

void ExploreExperiment::experiment_step(vector<double>& obs,
										int& action,
										bool& is_next,
										bool& fetch_action,
										SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper,
					 experiment_state);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper,
					   experiment_state);
		break;
	}
}

void ExploreExperiment::set_action(int action,
								   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_set_action(action,
						   experiment_state);
		break;
	}
}

void ExploreExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper,
						  experiment_state);
		break;
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper,
							experiment_state);
		break;
	}
}

void ExploreExperiment::backprop(double target_val,
								 ExploreExperimentHistory* history,
								 SolutionWrapper* wrapper) {
	if (this->last_num_instances.size() >= LAST_NUM_TRACK) {
		this->sum_num_instances -= this->last_num_instances.front();
		this->last_num_instances.pop_front();
	}
	this->last_num_instances.push_back(history->num_instances);
	this->sum_num_instances += history->num_instances;

	switch (this->state) {
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
	}
}
