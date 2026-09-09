#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "abstract_node.h"
#include "action_network.h"
#include "action_node.h"
#include "globals.h"
#include "minesweeper.h"
#include "obs_network.h"
#include "predict_network.h"
#include "scope.h"
#include "score_network.h"
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

	ProblemType* problem_type = new TypeMinesweeper();

	string filename;
	SolutionWrapper* solution_wrapper;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}
	solution_wrapper = new SolutionWrapper(
		"saves/",
		filename);

	Scope* scope = solution_wrapper->solution->starting_scope;

	Problem* problem = problem_type->get_problem();
	solution_wrapper->problem = problem;

	Eigen::VectorXf state;
	state.resize(scope->num_states);
	state.setConstant(0.0);

	{
		vector<double> obs = problem->get_observations();

		scope->start_obs_network->activate(state,
										   obs);
	}

	{
		int action = MINESWEEPER_ACTION_RIGHT;

		ActionNode* action_node = scope->generic_action_nodes[action];

		action_node->action_network->activate(state);

		problem->perform_action(action);

		action_node->predict_network->activate(state);
	}

	{
		int action = MINESWEEPER_ACTION_RIGHT;

		ActionNode* action_node = scope->generic_action_nodes[action];

		action_node->action_network->activate(state);

		problem->perform_action(action);

		action_node->predict_network->activate(state);
	}

	{
		int action = MINESWEEPER_ACTION_FLAG;

		ActionNode* action_node = scope->generic_action_nodes[action];

		action_node->action_network->activate(state);

		problem->perform_action(action);

		action_node->predict_network->activate(state);
	}

	scope->end_score_network->activate(state);
	double signal = scope->end_score_network->output->acti_vals(0);
	cout << "signal: " << signal << endl;

	double target_val = problem->score_result();
	target_val -= 0.0001 * solution_wrapper->run_num_actions;
	cout << "target_val: " << target_val << endl;

	problem->print();

	delete problem;

	delete problem_type;
	delete solution_wrapper;

	cout << "Done" << endl;
}
