#include "experiment.h"

#include "solution_wrapper.h"

using namespace std;

void Experiment::check_activate(AbstractNode* experiment_node,
								vector<double>& obs,
								SolutionWrapper* wrapper) {
	ExperimentHistory* history;
	map<Experiment*, ExperimentHistory*>::iterator it =
		wrapper->experiment_histories.find(this);
	if (it == wrapper->experiment_histories.end()) {
		history = new ExperimentHistory(this);
		wrapper->experiment_histories[this] = history;
	} else {
		history = it->second;
	}

	switch (this->state) {
	case EXPERIMENT_STATE_EXPLORE:
		explore_check_activate(obs,
							   wrapper,
							   history);
		break;
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_check_activate(obs,
								 wrapper,
								 history);
		break;
	case EXPERIMENT_STATE_RAMP:
	case EXPERIMENT_STATE_MEASURE:
		ramp_check_activate(obs,
							wrapper,
							history);
		break;
	}
}

void Experiment::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();
	switch (this->state) {
	case EXPERIMENT_STATE_EXPLORE:
		explore_step(obs,
					 action,
					 is_next,
					 fetch_action,
					 wrapper,
					 experiment_state);
		break;
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_step(obs,
					   action,
					   is_next,
					   wrapper,
					   experiment_state);
		break;
	case EXPERIMENT_STATE_RAMP:
	case EXPERIMENT_STATE_MEASURE:
		ramp_step(obs,
				  action,
				  is_next,
				  wrapper,
				  experiment_state);
		break;
	}
}

void Experiment::set_action(int action,
							SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();
	switch (this->state) {
	case EXPERIMENT_STATE_EXPLORE:
		explore_set_action(action,
						   experiment_state);
		break;
	}
}

void Experiment::experiment_exit_step(SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];
	switch (this->state) {
	case EXPERIMENT_STATE_EXPLORE:
		explore_exit_step(wrapper,
						  experiment_state);
		break;
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_exit_step(wrapper,
							experiment_state);
		break;
	case EXPERIMENT_STATE_RAMP:
	case EXPERIMENT_STATE_MEASURE:
		ramp_exit_step(wrapper,
					   experiment_state);
		break;
	}
}

void Experiment::backprop(double target_val,
						  ExperimentHistory* history,
						  SolutionWrapper* wrapper,
						  set<Scope*>& updated_scopes) {
	switch (this->state) {
	case EXPERIMENT_STATE_EXPLORE:
		explore_backprop(target_val,
						 history,
						 wrapper);
		break;
	case EXPERIMENT_STATE_TRAIN_NEW:
		train_new_backprop(target_val,
						   history,
						   wrapper);
		break;
	case EXPERIMENT_STATE_RAMP:
	case EXPERIMENT_STATE_MEASURE:
		ramp_backprop(target_val,
					  history,
					  wrapper,
					  updated_scopes);
		break;
	}
}
