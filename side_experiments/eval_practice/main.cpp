#include <iostream>

using namespace std;

#include "eval_experiment.h"
#include "eval_test.h"
#include "globals.h"
#include "scope.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeEvalTest();

	Scope* scope = new Scope();
	EvalExperiment* experiment = new EvalExperiment(scope);
	while (true) {
		Problem* problem = problem_type->get_problem();

		experiment->activate(problem);

		double target_val = problem->score_result();

		experiment->backprop(target_val);

		if (experiment->result != EXPERIMENT_RESULT_NA) {
			break;
		}
	}

	cout << "Done" << endl;
}
