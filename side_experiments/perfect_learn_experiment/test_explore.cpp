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

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPLORE_SAMPLES = 10;
#else
const int NUM_EXPLORE_SAMPLES = 100;
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
	} else {
		filename = "main.txt";
	}
	solution = new Solution(
		"saves/",
		filename);

	double best_surprise = 0.0;
	vector<int> best_actions;

	geometric_distribution<int> geo_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
	for (int iter_index = 0; iter_index < NUM_EXPLORE_SAMPLES; iter_index++) {
		Problem* problem = problem_type->get_problem();

		vector<double> obs = problem->get_observations();

		Eigen::VectorXf state;
		state.resize(NUM_STATES);
		state.setConstant(0.0);

		solution->obs_network->activate(state,
										obs);

		solution->score_network->activate(state);
		double existing_predicted = solution->score_network->output->acti_vals(0);

		int num_steps = geo_distribution(generator);

		vector<int> actions;
		for (int s_index = 0; s_index < num_steps; s_index++) {
			int action = action_distribution(generator);

			solution->action_networks[action]->activate(state);

			actions.push_back(action);
		}

		solution->score_network->activate(state);
		double new_predicted = solution->score_network->output->acti_vals(0);

		double surprise = new_predicted - existing_predicted;
		if (surprise > best_surprise) {
			best_surprise = surprise;
			best_actions = actions;
		}

		delete problem;
	}

	cout << "best_surprise: " << best_surprise << endl;
	cout << "best_actions:";
	for (int a_index = 0; a_index < (int)best_actions.size(); a_index++) {
		cout << " " << best_actions[a_index];
	}
	cout << endl;

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
