#include "helpers.h"

using namespace std;

#include "experiment.h"
#include "minesweeper.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

double result_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	double sum_vals = 0.0;

	for (int i_index = 0; i_index < 4000; i_index++) {
		Problem* problem = problem_type->get_problem();

		wrapper->init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * wrapper->num_actions;

		wrapper->end();

		sum_vals += target_val;

		delete problem;
	}

	delete problem_type;

	return sum_vals/4000.0;
}

double clean_result_helper(SolutionWrapper* wrapper) {
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
										wrapper);
			} else {
				wrapper->node_context.back()->result_step(obs,
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

	double target_val = copy_problem->score_result();
	target_val -= 0.0001 * wrapper->result_num_actions;

	delete copy_problem;

	return target_val;
}
