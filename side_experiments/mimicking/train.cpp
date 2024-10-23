// TODO: need to try LSTMs

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_network.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "nn_helpers.h"
#include "problem.h"
#include "sample.h"
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

	problem_type = new TypeMinesweeper();

	vector<Sample*> samples;
	for (int s_index = 0; s_index < 200; s_index++) {
		samples.push_back(new Sample(s_index));
	}

	StateNetwork* state_network = new StateNetwork(problem_type->num_obs(),
												   problem_type->num_possible_actions() + 1,
												   NUM_STATES);
	ActionNetwork* action_network = new ActionNetwork(NUM_STATES,
													  problem_type->num_possible_actions() + 1);

	train_network(samples,
				  state_network,
				  action_network);

	ofstream state_network_save_file;
	state_network_save_file.open("saves/state_network.txt");
	state_network->save(state_network_save_file);
	state_network_save_file.close();

	ofstream action_network_save_file;
	action_network_save_file.open("saves/action_network.txt");
	action_network->save(action_network_save_file);
	action_network_save_file.close();

	for (int s_index = 0; s_index < 200; s_index++) {
		delete samples[s_index];
	}
	delete state_network;
	delete action_network;

	delete problem_type;

	cout << "Done" << endl;
}
