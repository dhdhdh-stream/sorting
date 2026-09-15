// - if can predict sequence in a spot to be good, then predict should handle it
// - if look specifically for spots where predict bad, then could be lottery
//   - can't distinguish between not understanding and luck without focusing

// - so chase something that's potentially lucky?
//   - or chase something that's potentially lucky plus potentially don't understand
//     - probably better to chase after both

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
const int NUM_EXPLORE_SAMPLES = 20;
#else
const int NUM_EXPLORE_SAMPLES = 400;
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

		int num_steps = geo_distribution(generator);

		vector<int> actions;
		for (int s_index = 0; s_index < num_steps; s_index++) {
			int action = action_distribution(generator);

			problem->perform_action(action);

			solution->action_networks[action]->activate(state);

			actions.push_back(action);
		}

		solution->score_network->activate(state);
		double predicted = solution->score_network->output->acti_vals(0);

		double target_val = problem->score_result();

		double surprise = target_val - predicted;
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

	focus(problem_type,
		  best_actions,
		  solution);

	compare(problem_type,
			best_actions);

	vector<int> existing_actions;

	predict_decision_helper(problem_type,
							existing_actions,
							best_actions,
							solution);

	train_decision_helper(problem_type,
						  existing_actions,
						  best_actions);

	train_decision_on_state_helper(problem_type,
								   existing_actions,
								   best_actions,
								   solution);

	delete problem_type;
	delete solution;

	cout << "Done" << endl;
}
