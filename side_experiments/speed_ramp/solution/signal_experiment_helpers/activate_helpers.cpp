#include "signal_experiment.h"

#include <iostream>

#include "eval_experiment.h"
#include "explore_experiment.h"
#include "globals.h"
#include "helpers.h"
#include "refine_experiment.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

bool SignalExperiment::check_signal_activate(vector<double>& obs,
											 int& action,
											 bool& is_next,
											 bool& fetch_action,
											 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	SignalExperimentHistory* history;
	if (this->scope_context->signal_experiment_history == NULL) {
		history = new SignalExperimentHistory(this,
											  wrapper);
		this->scope_context->signal_experiment_history = history;
		wrapper->signal_experiment_histories.push_back({this, history});
		switch (this->state) {
		case SIGNAL_EXPERIMENT_STATE_INITIAL_C1:
		case SIGNAL_EXPERIMENT_STATE_INITIAL_C2:
		case SIGNAL_EXPERIMENT_STATE_INITIAL_C3:
		case SIGNAL_EXPERIMENT_STATE_INITIAL_C4:
			wrapper->has_explore = true;
			wrapper->explore_order_seen.push_back(this);
			break;
		}
	} else {
		history = this->scope_context->signal_experiment_history;
	}

	if (history->is_on) {
		/**
		 * - check pre
		 */
		if (scope_history->node_histories.size() == 0) {
			scope_history->signal_pre_obs.push_back(obs);

			if (scope_history->signal_pre_obs.size() <= this->pre_actions.size()) {
				if (!this->pre_action_initialized[scope_history->signal_pre_obs.size()-1]) {
					is_next = true;
					fetch_action = true;

					wrapper->num_actions++;

					SignalExperimentState* new_experiment_state = new SignalExperimentState(this);
					new_experiment_state->is_pre = true;
					new_experiment_state->index = scope_history->signal_pre_obs.size()-1;
					wrapper->experiment_context.back() = new_experiment_state;
				} else {
					action = this->pre_actions[scope_history->signal_pre_obs.size()-1];
					is_next = true;

					wrapper->num_actions++;
				}

				return true;
			} else {
				if (wrapper->should_explore && !wrapper->has_explore) {
					history->pre_obs.push_back(scope_history->signal_pre_obs);
				}
			}
		}

		/**
		 * - check post
		 */
		if (wrapper->node_context.back() == NULL
				&& wrapper->experiment_context.back() == NULL) {
			scope_history->signal_post_obs.push_back(obs);

			if (scope_history->signal_post_obs.size() <= this->post_actions.size()) {
				if (!this->post_action_initialized[scope_history->signal_post_obs.size()-1]) {
					is_next = true;
					fetch_action = true;

					wrapper->num_actions++;

					SignalExperimentState* new_experiment_state = new SignalExperimentState(this);
					new_experiment_state->is_pre = false;
					new_experiment_state->index = scope_history->signal_post_obs.size()-1;
					wrapper->experiment_context.back() = new_experiment_state;
				} else {
					action = this->post_actions[scope_history->signal_post_obs.size()-1];
					is_next = true;

					wrapper->num_actions++;
				}

				return true;
			} else {
				if (history->pre_obs.size() > history->post_obs.size()) {
					if (wrapper->has_explore) {
						history->post_obs.push_back(scope_history->signal_post_obs);

						history->sum_signal_vals.push_back(0.0);
						history->sum_counts.push_back(0);

						/**
						 * - don't include current layer
						 */
						for (int i_index = 0; i_index < (int)wrapper->scope_histories.size()-1; i_index++) {
							Scope* scope = wrapper->scope_histories[i_index]->scope;
							if (scope->pre_default_signal != NULL) {
								if (scope->signal_experiment_history == NULL
										|| !scope->signal_experiment_history->is_on) {
									wrapper->scope_histories[i_index]->signal_experiment_callbacks
										.push_back(history);
									wrapper->scope_histories[i_index]->signal_experiment_instance_indexes
										.push_back((int)history->sum_signal_vals.size()-1);
								}
							}
						}
					} else {
						history->pre_obs.pop_back();
					}
				}
			}
		}
	} else {
		/**
		 * - check pre
		 */
		if (scope_history->node_histories.size() == 0) {
			scope_history->signal_pre_obs.push_back(obs);

			if (scope_history->signal_pre_obs.size() <= this->scope_context->signal_pre_actions.size()) {
				action = this->scope_context->signal_pre_actions[scope_history->signal_pre_obs.size()-1];
				is_next = true;

				wrapper->num_actions++;

				return true;
			} else {
				if (wrapper->should_explore && !wrapper->has_explore) {
					history->pre_obs.push_back(scope_history->signal_pre_obs);
				}
			}
		}

		/**
		 * - check post
		 */
		if (wrapper->node_context.back() == NULL
				&& wrapper->experiment_context.back() == NULL) {
			scope_history->signal_post_obs.push_back(obs);

			if (scope_history->signal_post_obs.size() <= this->scope_context->signal_post_actions.size()) {
				action = this->scope_context->signal_post_actions[scope_history->signal_post_obs.size()-1];
				is_next = true;

				wrapper->num_actions++;

				return true;
			} else {
				if (this->scope_context->pre_default_signal != NULL) {
					scope_history->signal_initialized = true;
					scope_history->signal_val = calc_signal(scope_history);

					for (int e_index = 0; e_index < (int)scope_history->explore_experiment_callbacks.size(); e_index++) {
						ExploreExperimentHistory* explore_experiment_history = scope_history->explore_experiment_callbacks[e_index];
						int instance_index = scope_history->explore_experiment_instance_indexes[e_index];

						explore_experiment_history->sum_signal_vals[instance_index] += scope_history->signal_val;
						explore_experiment_history->sum_counts[instance_index]++;
					}
					for (int e_index = 0; e_index < (int)scope_history->refine_experiment_callbacks.size(); e_index++) {
						RefineExperimentHistory* refine_experiment_history = scope_history->refine_experiment_callbacks[e_index];
						int instance_index = scope_history->refine_experiment_instance_indexes[e_index];

						refine_experiment_history->sum_signal_vals[instance_index] += scope_history->signal_val;
						refine_experiment_history->sum_counts[instance_index]++;
					}
					for (int e_index = 0; e_index < (int)scope_history->signal_experiment_callbacks.size(); e_index++) {
						SignalExperimentHistory* signal_experiment_history = scope_history->signal_experiment_callbacks[e_index];
						int instance_index = scope_history->signal_experiment_instance_indexes[e_index];

						signal_experiment_history->sum_signal_vals[instance_index] += scope_history->signal_val;
						signal_experiment_history->sum_counts[instance_index]++;
					}
				}
				// if (scope_history->explore_experiment_callbacks.size() > 0
				// 		|| scope_history->signal_experiment_callbacks.size() > 0) {
				// 	double signal = calc_signal(scope_history);
				// 	for (int e_index = 0; e_index < (int)scope_history->explore_experiment_callbacks.size(); e_index++) {
				// 		ExploreExperimentHistory* explore_experiment_history = scope_history->explore_experiment_callbacks[e_index];
				// 		int instance_index = scope_history->explore_experiment_instance_indexes[e_index];

				// 		explore_experiment_history->sum_signal_vals[instance_index] += signal;
				// 		explore_experiment_history->sum_counts[instance_index]++;
				// 	}
				// 	for (int e_index = 0; e_index < (int)scope_history->signal_experiment_callbacks.size(); e_index++) {
				// 		SignalExperimentHistory* signal_experiment_history = scope_history->signal_experiment_callbacks[e_index];
				// 		int instance_index = scope_history->signal_experiment_instance_indexes[e_index];

				// 		signal_experiment_history->sum_signal_vals[instance_index] += signal;
				// 		signal_experiment_history->sum_counts[instance_index]++;
				// 	}
				// }

				if (history->pre_obs.size() > history->post_obs.size()) {
					if (wrapper->has_explore) {
						history->post_obs.push_back(scope_history->signal_post_obs);

						history->sum_signal_vals.push_back(0.0);
						history->sum_counts.push_back(0);

						/**
						 * - don't include current layer
						 */
						for (int i_index = 0; i_index < (int)wrapper->scope_histories.size()-1; i_index++) {
							Scope* scope = wrapper->scope_histories[i_index]->scope;
							if (scope->pre_default_signal != NULL) {
								if (scope->signal_experiment_history == NULL
										|| !scope->signal_experiment_history->is_on) {
									wrapper->scope_histories[i_index]->signal_experiment_callbacks
										.push_back(history);
									wrapper->scope_histories[i_index]->signal_experiment_instance_indexes
										.push_back((int)history->sum_signal_vals.size()-1);
								}
							}
						}
					} else {
						history->pre_obs.pop_back();
					}
				}
			}
		}
	}

	return false;
}

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
			history->sum_signal_vals[i_index] += target_val;
			history->sum_counts[i_index]++;

			double average_val = history->sum_signal_vals[i_index]
				/ (double)history->sum_counts[i_index];

			if (this->new_explore_pre_obs.size() >= EXPLORE_SAMPLES) {
				uniform_int_distribution<int> distribution(0, this->new_explore_pre_obs.size()-1);
				int index = distribution(generator);
				this->new_explore_pre_obs[index] = history->pre_obs[i_index];
				this->new_explore_post_obs[index] = history->post_obs[i_index];
				this->new_explore_scores[index] = average_val;
			} else {
				this->new_explore_pre_obs.push_back(history->pre_obs[i_index]);
				this->new_explore_post_obs.push_back(history->post_obs[i_index]);
				this->new_explore_scores.push_back(average_val);
			}
		}
	} else {
		this->existing_scores.push_back(target_val);

		for (int i_index = 0; i_index < (int)history->pre_obs.size(); i_index++) {
			history->sum_signal_vals[i_index] += target_val;
			history->sum_counts[i_index]++;

			double average_val = history->sum_signal_vals[i_index]
				/ (double)history->sum_counts[i_index];

			if (this->existing_explore_pre_obs.size() >= EXPLORE_SAMPLES) {
				uniform_int_distribution<int> distribution(0, this->existing_explore_pre_obs.size()-1);
				int index = distribution(generator);
				this->existing_explore_pre_obs[index] = history->pre_obs[i_index];
				this->existing_explore_post_obs[index] = history->post_obs[i_index];
				this->existing_explore_scores[index] = average_val;
			} else {
				this->existing_explore_pre_obs.push_back(history->pre_obs[i_index]);
				this->existing_explore_post_obs.push_back(history->post_obs[i_index]);
				this->existing_explore_scores.push_back(average_val);
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
	case SIGNAL_EXPERIMENT_STATE_WRAPUP:
		wrapup_backprop();
		break;
	}
}
