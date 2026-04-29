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

bool simulate_helper(SolutionWrapper* wrapper) {
	ProblemType* problem_type = new TypeMinesweeper();

	ScopeHistory* scope_history = new ScopeHistory(wrapper->scope_histories.back()->scope);
	wrapper->result_scope_histories.push_back(scope_history);
	wrapper->result_node_context.push_back(wrapper->node_context.back());
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
	case EXPERIMENT_TYPE_DAMAGE:
		{
			DamageState* original_experiment_state = (DamageState*)wrapper->experiment_context.back();
			DamageState* new_experiment_state = new DamageState((Damage*)(original_experiment_state->experiment));
			new_experiment_state->step_index = original_experiment_state->step_index;
			wrapper->result_experiment_context.push_back(new_experiment_state);
		}
		break;
	}			

	wrapper->result_num_actions = wrapper->num_actions;

	Problem* copy_problem = wrapper->problem->copy_snapshot();

	double starting_score = copy_problem->score_result();

	while (true) {
		vector<double> obs = copy_problem->get_observations();

		int action;
		bool is_next = false;
		bool is_done = false;
		bool fetch_action = false;
		while (!is_next) {
			if (wrapper->result_experiment_context.back() == NULL) {
				is_next = true;
				is_done = true;
			} else {
				AbstractExperiment* experiment = wrapper->result_experiment_context.back()->experiment;
				experiment->result_step(obs,
										action,
										is_next,
										fetch_action,
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

	double ending_score = copy_problem->score_result();

	bool hit_mine;
	if (ending_score < starting_score) {
		hit_mine = true;
	} else {
		hit_mine = false;
	}

	delete wrapper->result_scope_histories[0];

	wrapper->result_scope_histories.clear();
	wrapper->result_node_context.clear();
	wrapper->result_experiment_context.clear();

	delete copy_problem;

	delete problem_type;

	return hit_mine;
}
