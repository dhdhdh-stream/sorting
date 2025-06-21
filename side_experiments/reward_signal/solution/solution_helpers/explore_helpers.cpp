#include "solution_helpers.h"

#include "globals.h"
#include "scope.h"
#include "simple.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int NUM_EXPLORE_GATHER = 4000;

void explore_helper(SolutionWrapper* solution_wrapper) {
	ProblemType* problem_type = new TypeSimple();

	while (true) {
		Problem* problem = problem_type->get_problem();

		solution_wrapper->explore_init();

		while (true) {
			vector<double> obs = problem->get_observations();

			tuple<bool,bool,int> next = solution_wrapper->explore_step(obs);
			if (get<0>(next)) {
				break;
			} else if (get<1>(next)) {
				uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
				int new_action = action_distribution(generator);

				solution_wrapper->explore_set_action(new_action);

				problem->perform_action(new_action);
			} else {
				problem->perform_action(get<2>(next));
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;

		solution_wrapper->explore_end(target_val);

		delete problem;

		if (solution_wrapper->solution->scopes[0]->explore_scope_histories.size() >= NUM_EXPLORE_GATHER) {
			break;
		}
	}
}
