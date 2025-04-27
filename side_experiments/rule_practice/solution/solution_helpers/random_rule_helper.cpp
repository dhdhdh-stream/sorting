#include "solution_helpers.h"

#include "condition_action_equals.h"
#include "condition_obs_greater_than.h"
#include "condition_obs_less_than.h"
#include "condition_obs_within.h"
#include "condition_was_start.h"
#include "globals.h"
#include "problem.h"
#include "rule.h"

using namespace std;

Rule* random_rule() {
	Problem* problem = problem_type->get_problem();

	uniform_int_distribution<int> length_distribution(1, 49);
	int length = length_distribution(generator);

	vector<vector<double>> obs_history;
	vector<int> move_history;

	uniform_int_distribution<int> move_distribution(0, problem_type->num_possible_actions()-1);
	for (int l_index = 0; l_index < length; l_index++) {
		vector<double> obs = problem->get_observations();
		obs_history.push_back(obs);

		int move = move_distribution(generator);
		problem->perform_action(Action(move));
		move_history.push_back(move);
	}

	Rule* rule = new Rule();

	uniform_int_distribution<int> random_distribution(0, move_history.size()-1);
	int random_index = random_distribution(generator);

	rule->move = move_history[random_index];

	geometric_distribution<int> num_conditions_distribution(0.5);
	int num_conditions = 1 + num_conditions_distribution(generator);

	uniform_int_distribution<int> condition_type_distribution(0, 4);
	geometric_distribution<int> back_distribution(0.5);
	for (int c_index = 0; c_index < num_conditions; c_index++) {
		int condition_type = condition_type_distribution(generator);
		int back_iters = back_distribution(generator);
		switch (condition_type) {
		case 0:
			if (random_index - back_iters >= 0) {
				ConditionObsGreaterThan* condition = new ConditionObsGreaterThan(
					back_iters + 1,
					obs_history[random_index - back_iters][0]);
				rule->conditions.push_back(condition);
			} else {
				ConditionWasStart* condition = new ConditionWasStart(random_index + 1);
				rule->conditions.push_back(condition);
			}
			break;
		case 1:
			if (random_index - back_iters >= 0) {
				ConditionObsLessThan* condition = new ConditionObsLessThan(
					back_iters + 1,
					obs_history[random_index - back_iters][0]);
				rule->conditions.push_back(condition);
			} else {
				ConditionWasStart* condition = new ConditionWasStart(random_index + 1);
				rule->conditions.push_back(condition);
			}
			break;
		case 2:
			if (random_index - back_iters >= 0) {
				ConditionObsWithin* condition = new ConditionObsWithin(
					back_iters + 1,
					obs_history[random_index - back_iters][0],
					obs_history[random_index - back_iters][0]);
				rule->conditions.push_back(condition);
			} else {
				ConditionWasStart* condition = new ConditionWasStart(random_index + 1);
				rule->conditions.push_back(condition);
			}
			break;
		case 3:
		case 4:
			if (random_index - back_iters > 0) {
				ConditionActionEquals* condition = new ConditionActionEquals(
					back_iters + 1,
					move_history[random_index - back_iters - 1]);
				rule->conditions.push_back(condition);
			} else {
				ConditionWasStart* condition = new ConditionWasStart(random_index + 1);
				rule->conditions.push_back(condition);
			}
			break;
		}
	}

	return rule;
}
