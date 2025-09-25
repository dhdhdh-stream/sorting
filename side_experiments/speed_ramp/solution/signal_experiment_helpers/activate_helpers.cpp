// TODO: mark signal experiment explores

#include "signal_experiment.h"

#include <iostream>

#include "globals.h"
#include "solution_wrapper.h"

using namespace std;

void SignalExperiment::check_activate(AbstractNode* experiment_node,
									  bool is_branch,
									  SolutionWrapper* wrapper) {
	/**
	 * - unused
	 */
}

void SignalExperiment::experiment_step(std::vector<double>& obs,
									   int& action,
									   bool& is_next,
									   bool& fetch_action,
									   SolutionWrapper* wrapper) {
	/**
	 * - unused
	 */
}

void SignalExperiment::set_action(int action,
								  SolutionWrapper* wrapper) {
	SignalExperimentState* state = (SignalExperimentState*)wrapper->experiment_context.back();
	if (state->is_pre) {
		this->pre_action_initialized[state->index] = true;
		this->pre_actions[state->index] = action;
	} else {
		this->post_action_initialized[state->index] = true;
		this->post_actions[state->index] = action;
	}

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
	/**
	 * - simply delete state and recreate if needed
	 */
}

void SignalExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	/**
	 * - unused
	 */
}

void SignalExperiment::backprop(double target_val,
								SignalExperimentHistory* history,
								SolutionWrapper* wrapper) {
	if (history->is_on) {
		this->new_scores.push_back(target_val);

		for (int i_index = 0; i_index < (int)history->pre_obs.size(); i_index++) {
			if (history->signal_is_set[i_index]) {
				if (history->inner_is_explore[i_index] == history->outer_is_explore[i_index]) {
					if (history->inner_is_explore[i_index]) {
						if (this->new_explore_pre_obs.size() >= NEW_EXPLORE_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->new_explore_pre_obs.size()-1);
							int index = distribution(generator);
							this->new_explore_pre_obs[index] = history->pre_obs[i_index];
							this->new_explore_post_obs[index] = history->post_obs[i_index];
							this->new_explore_scores[index] = history->signal_vals[i_index];
						} else {
							this->new_explore_pre_obs.push_back(history->pre_obs[i_index]);
							this->new_explore_post_obs.push_back(history->post_obs[i_index]);
							this->new_explore_scores.push_back(history->signal_vals[i_index]);
						}
					} else {
						if (this->new_current_pre_obs.size() >= NEW_CURRENT_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->new_current_pre_obs.size()-1);
							int index = distribution(generator);
							this->new_current_pre_obs[index] = history->pre_obs[i_index];
							this->new_current_post_obs[index] = history->post_obs[i_index];
							this->new_current_scores[index] = history->signal_vals[i_index];
						} else {
							this->new_current_pre_obs.push_back(history->pre_obs[i_index]);
							this->new_current_post_obs.push_back(history->post_obs[i_index]);
							this->new_current_scores.push_back(history->signal_vals[i_index]);
						}
					}
				}
			} else {
				if (history->inner_is_explore[i_index] == wrapper->has_explore) {
					if (history->inner_is_explore[i_index]) {
						if (this->new_explore_pre_obs.size() >= NEW_EXPLORE_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->new_explore_pre_obs.size()-1);
							int index = distribution(generator);
							this->new_explore_pre_obs[index] = history->pre_obs[i_index];
							this->new_explore_post_obs[index] = history->post_obs[i_index];
							this->new_explore_scores[index] = history->signal_vals[i_index];
						} else {
							this->new_explore_pre_obs.push_back(history->pre_obs[i_index]);
							this->new_explore_post_obs.push_back(history->post_obs[i_index]);
							this->new_explore_scores.push_back(history->signal_vals[i_index]);
						}
					} else {
						if (this->new_current_pre_obs.size() >= NEW_CURRENT_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->new_current_pre_obs.size()-1);
							int index = distribution(generator);
							this->new_current_pre_obs[index] = history->pre_obs[i_index];
							this->new_current_post_obs[index] = history->post_obs[i_index];
							this->new_current_scores[index] = target_val;
						} else {
							this->new_current_pre_obs.push_back(history->pre_obs[i_index]);
							this->new_current_post_obs.push_back(history->post_obs[i_index]);
							this->new_current_scores.push_back(target_val);
						}
					}
				}
			}
		}
	} else {
		this->existing_scores.push_back(target_val);

		for (int i_index = 0; i_index < (int)history->pre_obs.size(); i_index++) {
			if (history->signal_is_set[i_index]) {
				if (history->inner_is_explore[i_index] == history->outer_is_explore[i_index]) {
					if (history->inner_is_explore[i_index]) {
						if (this->existing_explore_pre_obs.size() >= EXISTING_EXPLORE_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->existing_explore_pre_obs.size()-1);
							int index = distribution(generator);
							this->existing_explore_pre_obs[index] = history->pre_obs[i_index];
							this->existing_explore_post_obs[index] = history->post_obs[i_index];
							this->existing_explore_scores[index] = history->signal_vals[i_index];
						} else {
							this->existing_explore_pre_obs.push_back(history->pre_obs[i_index]);
							this->existing_explore_post_obs.push_back(history->post_obs[i_index]);
							this->existing_explore_scores.push_back(history->signal_vals[i_index]);
						}
					} else {
						if (this->existing_current_pre_obs.size() >= EXISTING_CURRENT_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->existing_current_pre_obs.size()-1);
							int index = distribution(generator);
							this->existing_current_pre_obs[index] = history->pre_obs[i_index];
							this->existing_current_post_obs[index] = history->post_obs[i_index];
							this->existing_current_scores[index] = history->signal_vals[i_index];
						} else {
							this->existing_current_pre_obs.push_back(history->pre_obs[i_index]);
							this->existing_current_post_obs.push_back(history->post_obs[i_index]);
							this->existing_current_scores.push_back(history->signal_vals[i_index]);
						}
					}
				}
			} else {
				if (history->inner_is_explore[i_index] == wrapper->has_explore) {
					if (history->inner_is_explore[i_index]) {
						if (this->existing_explore_pre_obs.size() >= EXISTING_EXPLORE_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->existing_explore_pre_obs.size()-1);
							int index = distribution(generator);
							this->existing_explore_pre_obs[index] = history->pre_obs[i_index];
							this->existing_explore_post_obs[index] = history->post_obs[i_index];
							this->existing_explore_scores[index] = history->signal_vals[i_index];
						} else {
							this->existing_explore_pre_obs.push_back(history->pre_obs[i_index]);
							this->existing_explore_post_obs.push_back(history->post_obs[i_index]);
							this->existing_explore_scores.push_back(history->signal_vals[i_index]);
						}
					} else {
						if (this->existing_current_pre_obs.size() >= EXISTING_CURRENT_SAMPLES) {
							uniform_int_distribution<int> distribution(0, this->existing_current_pre_obs.size()-1);
							int index = distribution(generator);
							this->existing_current_pre_obs[index] = history->pre_obs[i_index];
							this->existing_current_post_obs[index] = history->post_obs[i_index];
							this->existing_current_scores[index] = target_val;
						} else {
							this->existing_current_pre_obs.push_back(history->pre_obs[i_index]);
							this->existing_current_post_obs.push_back(history->post_obs[i_index]);
							this->existing_current_scores.push_back(target_val);
						}
					}
				}
			}
		}
	}

	switch (this->state) {
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
	case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
		initial_backprop(target_val,
						 history,
						 wrapper);
		break;
	case SIGNAL_EXPERIMENT_STATE_RAMP:
		ramp_backprop(target_val,
					  history,
					  wrapper);
		break;
	case SIGNAL_EXPERIMENT_STATE_GATHER:
		gather_backprop(target_val,
						history,
						wrapper);
		break;
	case SIGNAL_EXPERIMENT_STATE_WRAPUP:
		wrapup_backprop();
		break;
	}
}
