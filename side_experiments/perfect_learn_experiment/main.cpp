#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "minesweeper.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

#if defined(MDEBUG) && MDEBUG
const int EPOCH_NUM_ITERS = 10;
#else
const int EPOCH_NUM_ITERS = 100000;
#endif /* MDEBUG */

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeMinesweeper();

	string filename;
	Solution* solution;
	if (argc > 1) {
		filename = argv[1];
		solution = new Solution(
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		solution = new Solution(problem_type->num_obs(),
								problem_type->num_possible_actions());
	}

	geometric_distribution<int> geo_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
	while (true) {
		for (int iter_index = 0; iter_index < EPOCH_NUM_ITERS; iter_index++) {
			Problem* problem = problem_type->get_problem();

			vector<double> obs = problem->get_observations();

			int num_steps = geo_distribution(generator);

			vector<int> actions;
			for (int s_index = 0; s_index < num_steps; s_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);

				actions.push_back(action);
			}

			double target_val = problem->score_result();

			train_helper(obs,
						 actions,
						 target_val,
						 solution);

			delete problem;

			if (iter_index%10000 == 0) {
				cout << iter_index << endl;
			}
		}

		solution->save("saves/", filename);
	}

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
