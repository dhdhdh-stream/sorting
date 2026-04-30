#include "solution_helpers.h"

#include <iostream>

#include "damage.h"
#include "eval_experiment.h"
#include "explore_experiment.h"
#include "globals.h"
#include "minesweeper.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

double clean_result_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	for (int l_index = 0; l_index < (int)wrapper->scope_histories.size(); l_index++) {
		ScopeHistory* scope_history = new ScopeHistory(wrapper->scope_histories[l_index]->scope);
		wrapper->result_scope_histories.push_back(scope_history);
		wrapper->result_node_context.push_back(wrapper->node_context[l_index]);
		if (wrapper->experiment_context[l_index] == NULL) {
			wrapper->result_experiment_context.push_back(NULL);
		} else {
			AbstractExperiment* abstract_experiment = wrapper->experiment_context.back()->experiment;
			switch (abstract_experiment->type) {
			case EXPERIMENT_TYPE_EXPLORE:
				{
					ExploreExperimentState* original_experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
					ExploreExperimentState* new_experiment_state = new ExploreExperimentState((ExploreExperiment*)(original_experiment_state->experiment));
					new_experiment_state->step_index = original_experiment_state->step_index;
					wrapper->result_experiment_context.push_back(new_experiment_state);
				}
				break;
			case EXPERIMENT_TYPE_EVAL:
				{
					EvalExperimentState* original_experiment_state = (EvalExperimentState*)wrapper->experiment_context.back();
					EvalExperimentState* new_experiment_state = new EvalExperimentState((EvalExperiment*)(original_experiment_state->experiment));
					new_experiment_state->step_index = original_experiment_state->step_index;
					wrapper->result_experiment_context.push_back(new_experiment_state);
				}
				break;
			case EXPERIMENT_TYPE_DAMAGE:
				{
					DamageState* original_experiment_state = (DamageState*)wrapper->experiment_context.back();
					DamageState* new_experiment_state = new DamageState((Damage*)(original_experiment_state->experiment));
					new_experiment_state->step_index = original_experiment_state->step_index;
					wrapper->result_experiment_context.push_back(new_experiment_state);
				}
				break;
			}
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
	target_val -= 0.0001 * wrapper->result_num_actions;

	delete wrapper->result_scope_histories[0];

	wrapper->result_scope_histories.clear();
	wrapper->result_node_context.clear();
	wrapper->result_experiment_context.clear();

	delete copy_problem;

	delete problem_type;

	return target_val;
}
