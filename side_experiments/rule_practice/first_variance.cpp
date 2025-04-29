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

	int num_good = 0;
	for (int e_index = 0; e_index < 1000; e_index++) {
		if (e_index % 10 == 0) {
			cout << e_index << endl;
		}

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

		// if (potential_percentage > MIN_POTENTIAL_PERCENTAGE
		// 		&& new_average_val > solution.average_val
		// 		&& new_top_5_percentile >= solution.top_5_percentile
		// 		&& new_top_5_percentile_average_val >= solution.top_5_percentile_average_val) {
		// 	cout << "new_average_val: " << new_average_val << endl;
		// 	cout << "new_top_5_percentile: " << new_top_5_percentile << endl;
		// 	cout << "new_top_5_percentile_average_val: " << new_top_5_percentile_average_val << endl;
		// }
		if (new_average_val > 0.0) {
			cout << "new_average_val: " << new_average_val << endl;
			cout << "new_top_5_percentile: " << new_top_5_percentile << endl;
			cout << "new_top_5_percentile_average_val: " << new_top_5_percentile_average_val << endl;

			num_good++;
		}

		delete potential_rule;
	}

	cout << "num_good: " << num_good << endl;

	delete problem_type;

	cout << "Done" << endl;
}
