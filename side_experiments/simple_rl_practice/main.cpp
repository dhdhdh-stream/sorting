#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"
#include "nn_helpers.h"
#include "simple_rl_practice.h"

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

	problem_type = new TypeSimpleRLPractice();

	vector<Network*> networks(problem_type->num_possible_actions() + 1);
	for (int a_index = 0; a_index < (int)problem_type->num_possible_actions() + 1; a_index++) {
		networks[a_index] = new Network(problem_type->num_obs());
	}

	train_rl(networks);

	for (int n_index = 0; n_index < (int)networks.size(); n_index++) {
		delete networks[n_index];
	}

	delete problem_type;

	cout << "Seed: " << seed << endl;

	cout << "Done" << endl;
}
