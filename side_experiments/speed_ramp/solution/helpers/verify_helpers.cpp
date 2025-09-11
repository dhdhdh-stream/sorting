#include "helpers.h"

#include <iostream>

#include "abstract_experiment.h"
#include "abstract_node.h"
#include "problem.h"
#include "scope.h"
#include "simpler.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_MEASURE = 20;
#else
const int NUM_MEASURE = 4000;
#endif /* MDEBUG */

void measure_score(SolutionWrapper* solution_wrapper) {
	ProblemType* problem_type = new TypeSimpler();

	double sum_scores = 0.0;
	for (int iter_index = 0; iter_index < NUM_MEASURE; iter_index++) {
		Problem* problem = problem_type->get_problem();

		solution_wrapper->init();

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = solution_wrapper->step(obs);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;

		solution_wrapper->end();

		sum_scores += target_val;

		delete problem;
	}

	double score_average = sum_scores / (double)NUM_MEASURE;
	cout << "score_average: " << score_average << endl;

	int explores_in_flight = 0;
	int evals_in_flight = 0;
	for (map<int, Scope*>::iterator scope_it = solution_wrapper->solution->scopes.begin();
			scope_it != solution_wrapper->solution->scopes.end(); scope_it++) {
		for (map<int, AbstractNode*>::iterator node_it = scope_it->second->nodes.begin();
				node_it != scope_it->second->nodes.end(); node_it++) {
			if (node_it->second->experiment != NULL) {
				switch (node_it->second->experiment->type) {
				case EXPERIMENT_TYPE_EXPLORE:
					explores_in_flight++;
					break;
				case EXPERIMENT_TYPE_EVAL:
					evals_in_flight++;
					break;
				}
			}
		}
	}
	cout << "explores_in_flight: " << explores_in_flight << endl;
	cout << "evals_in_flight: " << evals_in_flight << endl;

	delete problem_type;
}
