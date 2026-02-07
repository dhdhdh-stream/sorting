#include "solution_helpers.h"

#include <iostream>

#include "constants.h"
#include "experiment.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "sum_tree.h"
#include "utilities.h"

using namespace std;

void get_existing_result_init(SolutionWrapper* wrapper) {
	wrapper->result_num_actions = wrapper->result_num_actions;

	wrapper->sum_signals = wrapper->result_sum_signals;
	wrapper->signal_count = wrapper->result_signal_count;

	for (int l_index = 0; l_index < (int)wrapper->scope_histories.size(); l_index++) {
		ScopeHistory* scope_history = new ScopeHistory(wrapper->scope_histories[l_index]->scope);
		scope_history->pre_obs_history = wrapper->scope_histories[l_index]->pre_obs_history;
		wrapper->result_scope_histories.push_back(scope_history);
	}
	wrapper->result_node_context = wrapper->node_context;
	wrapper->result_experiment_context = vector<AbstractExperimentState*>(wrapper->experiment_context.size(), NULL);
}

void get_existing_result(SolutionWrapper* wrapper) {
	/**
	 * - already init
	 */

	Problem* copy_problem = wrapper->problem->copy_snapshot();

	while (true) {
		vector<double> obs = copy_problem->get_observations();

		int action;
		bool is_next = false;
		bool is_done = false;
		while (!is_next) {
			if (wrapper->result_node_context.back() == NULL
					&& wrapper->result_experiment_context.back() == NULL) {
				ScopeHistory* scope_history = wrapper->result_scope_histories.back();
				scope_history->post_obs_history = obs;

				for (int i_index = 0; i_index < (int)scope_history->experiment_callback_histories.size(); i_index++) {
					vector<double> input;
					input.insert(input.end(), scope_history->pre_obs_history.begin(), scope_history->pre_obs_history.end());
					input.insert(input.end(), scope_history->post_obs_history.begin(), scope_history->post_obs_history.end());

					Scope* scope = scope_history->scope;

					wrapper->curr_experiment->existing_pre_obs.push_back(scope_history->pre_obs_history);
					wrapper->curr_experiment->existing_post_obs.push_back(scope_history->post_obs_history);
					wrapper->curr_experiment->existing_signal_vals.push_back(scope->signal->activate(input));
				}

				if (wrapper->result_scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					if (wrapper->result_experiment_context[wrapper->result_experiment_context.size() - 2] != NULL) {
						AbstractExperiment* experiment = wrapper->result_experiment_context[wrapper->result_experiment_context.size() - 2]->experiment;
						experiment->result_experiment_exit_step(wrapper);
					} else {
						ScopeNode* scope_node = (ScopeNode*)wrapper->result_node_context[wrapper->result_node_context.size() - 2];
						scope_node->result_exit_step(wrapper);
					}
				}
			} else if (wrapper->result_experiment_context.back() != NULL) {
				AbstractExperiment* experiment = wrapper->result_experiment_context.back()->experiment;
				experiment->result_experiment_step(obs,
												   action,
												   is_next,
												   wrapper);
			} else {
				wrapper->result_node_context.back()->result_step(
					obs,
					action,
					is_next,
					wrapper);
			}
		}

		if (is_done) {
			break;
		} else {
			copy_problem->perform_action(action);
		}
	}

	delete wrapper->result_scope_histories[0];

	wrapper->result_scope_histories.clear();
	wrapper->result_node_context.clear();
	wrapper->result_experiment_context.clear();

	delete copy_problem;
}
