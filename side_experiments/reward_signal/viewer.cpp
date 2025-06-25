#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "globals.h"
#include "scope.h"
#include "simpler.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimpler();

	string filename;
	SolutionWrapper* solution_wrapper;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}
	solution_wrapper = new SolutionWrapper(
		problem_type->num_obs(),
		"saves/",
		filename);

	// {
	// 	Problem* problem = problem_type->get_problem();

	// 	solution_wrapper->init();

	// 	while (true) {
	// 		vector<double> obs = problem->get_observations();

	// 		pair<bool,int> next = solution_wrapper->step(obs);
	// 		if (next.first) {
	// 			break;
	// 		} else {
	// 			problem->perform_action(next.second);
	// 		}
	// 	}

	// 	double target_val = problem->score_result();
	// 	target_val -= 0.0001 * solution_wrapper->num_actions;
	// 	cout << "target_val: " << target_val << endl;

	// 	solution_wrapper->end();

	// 	problem->print();

	// 	cout << "solution_wrapper->num_actions: " << solution_wrapper->num_actions << endl;

	// 	delete problem;
	// }

	{
		Problem* problem = problem_type->get_problem();

		solution_wrapper->experiment_init();

		solution_wrapper->measure_match = true;

		while (true) {
			vector<double> obs = problem->get_observations();

			tuple<bool,bool,int> next = solution_wrapper->experiment_step(obs);
			if (get<0>(next)) {
				break;
			} else if (get<1>(next)) {
				uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
				int new_action = action_distribution(generator);

				solution_wrapper->set_action(new_action);

				problem->perform_action(new_action);
			} else {
				problem->perform_action(get<2>(next));
			}
		}

		double target_val = problem->score_result();
		target_val -= 0.0001 * solution_wrapper->num_actions;

		cout << "t_scores: " << endl;
		for (int t_index = 0; t_index < (int)solution_wrapper->t_scores.size(); t_index++) {
			cout << solution_wrapper->t_scores[t_index] << endl;
		}
		is_match(solution_wrapper->t_scores);

		delete problem;
	}

	solution_wrapper->save_for_display("../", "display.txt");

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
