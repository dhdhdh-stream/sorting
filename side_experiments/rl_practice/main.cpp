#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "decision_network.h"
#include "eval_network.h"
#include "nn_helpers.h"
#include "rl_practice.h"
#include "simple_rl_practice.h"
#include "state_network.h"

using namespace std;

int seed;

default_random_engine generator;

ProblemType* problem_type;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	// problem_type = new TypeRLPractice();
	problem_type = new TypeSimpleRLPractice();

	DecisionNetwork* decision_network = new DecisionNetwork(
		problem_type->num_obs(),
		problem_type->num_possible_actions() + 1,
		RL_STATE_SIZE);
	StateNetwork* state_network = new StateNetwork(
		problem_type->num_obs(),
		problem_type->num_possible_actions() + 2,
		RL_STATE_SIZE);
	EvalNetwork* eval_network = new EvalNetwork(
		problem_type->num_obs(),
		problem_type->num_possible_actions() + 2,
		RL_STATE_SIZE);

	double average_val;
	train_rl(decision_network,
			 state_network,
			 eval_network,
			 average_val);

	delete decision_network;
	delete state_network;
	delete eval_network;

	delete problem_type;

	cout << "Done" << endl;
}
