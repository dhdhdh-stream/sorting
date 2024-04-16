#include <iostream>

#include "globals.h"
#include "minesweeper.h"
#include "minesweeper_solution.h"

using namespace std;

int seed;

default_random_engine generator;

Problem* problem_type;
Solution* solution;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ManualSolution* manual_solution = new MinesweeperSolution();

	double sum_vals = 0.0;
	for (int i_index = 0; i_index < 2000; i_index++) {
		Problem* problem = new Minesweeper();

		while (true) {
			vector<double> obs = problem->get_observations();
			bool done;
			vector<Action> actions;
			manual_solution->step(obs,
								  done,
								  actions);

			for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
				problem->perform_action(actions[a_index]);
			}

			if (done) {
				break;
			}
		}

		double target_val = problem->score_result();
		sum_vals += target_val;

		delete problem;
	}

	cout << "average score: " << sum_vals/2000 << endl;

	delete manual_solution;

	cout << "Done" << endl;
}
