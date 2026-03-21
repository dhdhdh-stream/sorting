#include "helpers.h"

#include <iostream>

#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int UPDATE_ITERS = 10;
const int SAMPLES_PER_ITER = 2;
#else
const int UPDATE_ITERS = 100000;
const int SAMPLES_PER_ITER = 10;
#endif /* MDEBUG */

void existing_clean_result_helper(SolutionWrapper* wrapper,
								  double& result,
								  double& signal) {
	ProblemType* problem_type = new TypeMinesweeper();

	for (int l_index = 0; l_index < (int)wrapper->scope_histories.size(); l_index++) {
		ScopeHistory* scope_history = new ScopeHistory(wrapper->scope_histories[l_index]->scope);
		wrapper->result_scope_histories.push_back(scope_history);
		wrapper->result_node_context.push_back(wrapper->node_context[l_index]);
		if (wrapper->experiment_context[l_index] == NULL) {
			wrapper->result_experiment_context.push_back(NULL);
		} else {
			ExperimentState* original_experiment_state = (ExperimentState*)wrapper->experiment_context[l_index];
			ExperimentState* new_experiment_state = new ExperimentState((Experiment*)(original_experiment_state->experiment));
			new_experiment_state->step_index = original_experiment_state->step_index;
			wrapper->result_experiment_context.push_back(new_experiment_state);
		}
	}

	wrapper->result_num_actions = wrapper->num_actions;

	Problem* copy_problem = wrapper->problem->copy_snapshot();

	while (true) {
		vector<double> obs = copy_problem->get_observations();

		int action;
		bool is_next = false;
		bool is_done = false;
		bool fetch_action = false;
		while (!is_next) {
			if (wrapper->result_node_context.back() == NULL
					&& wrapper->result_experiment_context.back() == NULL) {
				if (wrapper->result_scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					if (wrapper->result_experiment_context[wrapper->result_experiment_context.size() - 2] != NULL) {
						AbstractExperiment* experiment = wrapper->result_experiment_context[wrapper->result_experiment_context.size() - 2]->experiment;
						experiment->result_exit_step(wrapper);
					} else {
						ScopeNode* scope_node = (ScopeNode*)wrapper->result_node_context[wrapper->result_node_context.size() - 2];
						scope_node->result_exit_step(wrapper);
					}
				}
			} else if (wrapper->result_experiment_context.back() != NULL) {
				AbstractExperiment* experiment = wrapper->result_experiment_context.back()->experiment;
				experiment->result_step(obs,
										action,
										is_next,
										fetch_action,
										wrapper);
			} else {
				wrapper->result_node_context.back()->result_step(obs,
																 action,
																 is_next,
																 wrapper);
			}
		}
		if (is_done) {
			break;
		} else if (fetch_action) {
			uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
			int new_action = action_distribution(generator);

			AbstractExperiment* experiment = wrapper->result_experiment_context.back()->experiment;
			experiment->result_set_action(new_action,
										  wrapper);

			copy_problem->perform_action(new_action);
		} else {
			copy_problem->perform_action(action);
		}
	}

	double target_val = copy_problem->score_result();
	// target_val -= 0.0001 * wrapper->result_num_actions;
	result = target_val;

	/**
	 * - simply only update existing signal on existing_clean_result_helper()
	 *   - ramp should hopefully provide enough variance
	 */
	{
		Scope* scope = wrapper->solution->scopes[0];

		vector<double> obs = copy_problem->get_observations();

		scope->simple_existing_signal->activate(obs);
		signal = scope->simple_existing_signal->output->acti_vals[0];

		if (scope->existing_obs_histories.size() < HISTORIES_NUM_SAVE) {
			scope->existing_obs_histories.push_back(obs);
			scope->existing_target_val_histories.push_back(target_val);

			if (scope->existing_obs_histories.size() >= HISTORIES_NUM_SAVE) {
				uniform_int_distribution<int> distribution(0, scope->existing_obs_histories.size()-1);
				for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
					int index = distribution(generator);

					scope->simple_existing_signal->activate(scope->existing_obs_histories[index]);

					double error = scope->existing_target_val_histories[index] - scope->simple_existing_signal->output->acti_vals[0];

					scope->simple_existing_signal->backprop(error);
				}
			}
		} else {
			scope->existing_obs_histories[scope->existing_history_index] = obs;
			scope->existing_target_val_histories[scope->existing_history_index] = target_val;
			scope->existing_history_index++;
			if (scope->existing_history_index >= HISTORIES_NUM_SAVE) {
				scope->existing_history_index = 0;
			}

			uniform_int_distribution<int> distribution(0, scope->existing_obs_histories.size()-1);
			for (int s_index = 0; s_index < SAMPLES_PER_ITER; s_index++) {
				int index = distribution(generator);

				scope->simple_existing_signal->activate(scope->existing_obs_histories[index]);

				double error = scope->existing_target_val_histories[index] - scope->simple_existing_signal->output->acti_vals[0];

				scope->simple_existing_signal->backprop(error);
			}
		}
	}

	delete wrapper->result_scope_histories[0];

	wrapper->result_scope_histories.clear();
	wrapper->result_node_context.clear();
	wrapper->result_experiment_context.clear();

	delete copy_problem;

	delete problem_type;
}

void explore_clean_result_helper(SolutionWrapper* wrapper,
								 double& result,
								 double& signal) {
	ProblemType* problem_type = new TypeMinesweeper();

	for (int l_index = 0; l_index < (int)wrapper->scope_histories.size(); l_index++) {
		ScopeHistory* scope_history = new ScopeHistory(wrapper->scope_histories[l_index]->scope);
		wrapper->result_scope_histories.push_back(scope_history);
		wrapper->result_node_context.push_back(wrapper->node_context[l_index]);
		if (wrapper->experiment_context[l_index] == NULL) {
			wrapper->result_experiment_context.push_back(NULL);
		} else {
			ExperimentState* original_experiment_state = (ExperimentState*)wrapper->experiment_context[l_index];
			ExperimentState* new_experiment_state = new ExperimentState((Experiment*)(original_experiment_state->experiment));
			new_experiment_state->step_index = original_experiment_state->step_index;
			wrapper->result_experiment_context.push_back(new_experiment_state);
		}
	}

	wrapper->result_num_actions = wrapper->num_actions;

	Problem* copy_problem = wrapper->problem->copy_snapshot();

	while (true) {
		vector<double> obs = copy_problem->get_observations();

		int action;
		bool is_next = false;
		bool is_done = false;
		bool fetch_action = false;
		while (!is_next) {
			if (wrapper->result_node_context.back() == NULL
					&& wrapper->result_experiment_context.back() == NULL) {
				if (wrapper->result_scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					if (wrapper->result_experiment_context[wrapper->result_experiment_context.size() - 2] != NULL) {
						AbstractExperiment* experiment = wrapper->result_experiment_context[wrapper->result_experiment_context.size() - 2]->experiment;
						experiment->result_exit_step(wrapper);
					} else {
						ScopeNode* scope_node = (ScopeNode*)wrapper->result_node_context[wrapper->result_node_context.size() - 2];
						scope_node->result_exit_step(wrapper);
					}
				}
			} else if (wrapper->result_experiment_context.back() != NULL) {
				AbstractExperiment* experiment = wrapper->result_experiment_context.back()->experiment;
				experiment->result_step(obs,
										action,
										is_next,
										fetch_action,
										wrapper);
			} else {
				wrapper->result_node_context.back()->result_step(obs,
																 action,
																 is_next,
																 wrapper);
			}
		}
		if (is_done) {
			break;
		} else if (fetch_action) {
			uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
			int new_action = action_distribution(generator);

			AbstractExperiment* experiment = wrapper->result_experiment_context.back()->experiment;
			experiment->result_set_action(new_action,
										  wrapper);

			copy_problem->perform_action(new_action);
		} else {
			copy_problem->perform_action(action);
		}
	}

	double target_val = copy_problem->score_result();
	// target_val -= 0.0001 * wrapper->result_num_actions;
	result = target_val;

	{
		Scope* scope = wrapper->solution->scopes[0];

		vector<double> obs = copy_problem->get_observations();

		scope->simple_explore_signal->activate(obs);
		signal = scope->simple_explore_signal->output->acti_vals[0];

		if (scope->explore_obs_histories.size() < HISTORIES_NUM_SAVE) {
			scope->explore_obs_histories.push_back(obs);
			scope->explore_target_val_histories.push_back(target_val);

			if (scope->explore_obs_histories.size() >= HISTORIES_NUM_SAVE) {
				uniform_int_distribution<int> distribution(0, scope->explore_obs_histories.size()-1);
				for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
					int index = distribution(generator);

					scope->simple_explore_signal->activate(scope->explore_obs_histories[index]);

					double error = scope->explore_target_val_histories[index] - scope->simple_explore_signal->output->acti_vals[0];

					scope->simple_explore_signal->backprop(error);
				}
			}
		} else {
			scope->explore_obs_histories[scope->explore_history_index] = obs;
			scope->explore_target_val_histories[scope->explore_history_index] = target_val;
			scope->explore_history_index++;
			if (scope->explore_history_index >= HISTORIES_NUM_SAVE) {
				scope->explore_history_index = 0;
			}

			uniform_int_distribution<int> distribution(0, scope->explore_obs_histories.size()-1);
			for (int s_index = 0; s_index < SAMPLES_PER_ITER; s_index++) {
				int index = distribution(generator);

				scope->simple_explore_signal->activate(scope->explore_obs_histories[index]);

				double error = scope->explore_target_val_histories[index] - scope->simple_explore_signal->output->acti_vals[0];

				scope->simple_explore_signal->backprop(error);
			}
		}
	}

	delete wrapper->result_scope_histories[0];

	wrapper->result_scope_histories.clear();
	wrapper->result_node_context.clear();
	wrapper->result_experiment_context.clear();

	delete copy_problem;

	delete problem_type;
}
