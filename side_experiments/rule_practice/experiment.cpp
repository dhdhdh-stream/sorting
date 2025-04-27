#include <iostream>

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

	Solution solution;
	solution.init();

	for (int e_index = 0; e_index < 10000; e_index++) {
		Rule* potential_rule = random_rule();
		double new_average_val;
		double new_top_5_percentile;
		double new_top_5_percentile_average_val;
		double potential_percentage;
		rule_experiment(solution.rules,
						potential_rule,
						new_average_val,
						new_top_5_percentile,
						new_top_5_percentile_average_val,
						potential_percentage);

		if (potential_percentage > MIN_POTENTIAL_PERCENTAGE
				&& new_average_val > solution.average_val
				&& new_top_5_percentile >= solution.top_5_percentile
				&& new_top_5_percentile_average_val >= solution.top_5_percentile_average_val) {
			solution.rules.push_back(potential_rule);
			solution.average_val = new_average_val;
			solution.top_5_percentile = new_top_5_percentile;
			solution.top_5_percentile_average_val = new_top_5_percentile_average_val;
		} else {
			delete potential_rule;
		}

		if (e_index % 10 == 0) {
			cout << e_index << endl;
			cout << "solution.rules.size(): " << solution.rules.size() << endl;
			cout << "solution.average_val: " << solution.average_val << endl;
			cout << "solution.top_5_percentile: " << solution.top_5_percentile << endl;
			cout << "solution.top_5_percentile_average_val: " << solution.top_5_percentile_average_val << endl;
			cout << endl;
		}

		if (e_index % 100 == 0) {
			solution.save("saves/", "main.txt");
		}
	}

	delete problem_type;

	cout << "Done" << endl;
}
