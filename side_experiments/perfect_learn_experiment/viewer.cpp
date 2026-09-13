#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_network.h"
#include "constants.h"
#include "minesweeper.h"
#include "obs_network.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

int seed;

default_random_engine generator;

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
	} else {
		filename = "main.txt";
	}
	solution = new Solution(
		"saves/",
		filename);

	vector<int> test_actions{0, 0, 1, 5};

	Problem* problem = problem_type->get_problem();

	Eigen::VectorXf state;
	state.resize(NUM_STATES);
	state.setConstant(0.0);

	vector<double> obs = problem->get_observations();
	solution->obs_network->activate(state,
									obs);

	solution->score_network->activate(state);
	double predicted = solution->score_network->output->acti_vals(0);
	cout << "predicted: " << predicted << endl;

	for (int a_index = 0; a_index < (int)test_actions.size(); a_index++) {
		problem->perform_action(test_actions[a_index]);

		solution->action_networks[test_actions[a_index]]->activate(state);

		solution->score_network->activate(state);
		double predicted = solution->score_network->output->acti_vals(0);
		cout << "predicted: " << predicted << endl;
	}

	double target_val = problem->score_result();
	cout << "target_val: " << target_val << endl;

	problem->print();

	delete problem;

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
