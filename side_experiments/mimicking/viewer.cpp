#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_network.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "problem.h"
#include "sample.h"

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

	// ifstream state_network_save_file;
	// state_network_save_file.open("saves/state_network.txt");
	// StateNetwork* state_network = new StateNetwork(state_network_save_file);
	// state_network_save_file.close();

	// ifstream action_network_save_file;
	// action_network_save_file.open("saves/action_network.txt");
	// ActionNetwork* action_network = new ActionNetwork(action_network_save_file);
	// action_network_save_file.close();

	// Sample sample(0);

	// vector<double> state_vals(NUM_STATES, 0.0);
	// for (int s_index = 0; s_index < (int)sample.actions.size(); s_index++) {
	// 	cout << "s_index: " << s_index << endl;

	// 	cout << "actions: " << sample.actions[s_index].move << endl;

	// 	cout << "obs:" << endl;
	// 	for (int x_index = 0; x_index < 5; x_index++) {
	// 		for (int y_index = 0; y_index < 5; y_index++) {
	// 			cout << sample.obs[s_index][5 * y_index + x_index] << " ";
	// 		}
	// 		cout << endl;
	// 	}
	// 	cout << "(" << sample.locations[s_index][12][0] << "," << sample.locations[s_index][12][1] << ")" << endl;

	// 	state_network->activate(sample.obs[s_index],
	// 							sample.actions[s_index].move,
	// 							state_vals);

	// 	action_network->activate(state_vals);
	// 	cout << "TERMINATE: " << action_network->output->acti_vals[0] << endl;
	// 	cout << "UP: " << action_network->output->acti_vals[1] << endl;
	// 	cout << "RIGHT: " << action_network->output->acti_vals[2] << endl;
	// 	cout << "DOWN: " << action_network->output->acti_vals[3] << endl;
	// 	cout << "LEFT: " << action_network->output->acti_vals[4] << endl;
	// 	cout << "CLICK: " << action_network->output->acti_vals[5] << endl;
	// 	cout << "FLAG: " << action_network->output->acti_vals[6] << endl;
	// 	cout << "DOUBLECLICK: " << action_network->output->acti_vals[7] << endl;

	// 	cout << endl;
	// }

	// delete state_network;
	// delete action_network;

	delete problem_type;

	cout << "Done" << endl;
}
