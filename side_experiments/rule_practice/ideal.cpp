#include <iostream>

#include "condition_obs_greater_than.h"
#include "condition_obs_less_than.h"
#include "constants.h"
#include "globals.h"
#include "rule.h"
#include "simple_problem.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeSimpleProblem();

	vector<Rule*> rules;

	{
		Rule* rule = new Rule();
		rule->conditions.push_back(
			new ConditionObsLessThan(1, 0.0));
		rule->move = 2;
		rules.push_back(rule);
	}

	// {
	// 	Rule* rule = new Rule();
	// 	rule->conditions.push_back(
	// 		new ConditionObsGreaterThan(1, 20.0));
	// 	rule->move = 2;
	// 	rules.push_back(rule);
	// }

	double new_average_val;
	double new_top_5_percentile;
	double new_top_5_percentile_average_val;
	rule_experiment(rules,
					new_average_val,
					new_top_5_percentile,
					new_top_5_percentile_average_val);

	cout << "new_average_val: " << new_average_val << endl;
	cout << "new_top_5_percentile: " << new_top_5_percentile << endl;
	cout << "new_top_5_percentile_average_val: " << new_top_5_percentile_average_val << endl;

	delete problem_type;

	cout << "Done" << endl;
}
