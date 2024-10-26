#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_network.h"
#include "constants.h"
#include "globals.h"
#include "lstm.h"
#include "minesweeper.h"
#include "nn_helpers.h"
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

	vector<Sample*> samples;
	for (int s_index = 0; s_index < 200; s_index++) {
		samples.push_back(new Sample(s_index));
	}

	// vector<LSTM*> memory_cells(NUM_STATES);
	// for (int s_index = 0; s_index < NUM_STATES; s_index++) {
	// 	memory_cells[s_index] = new LSTM(problem_type->num_obs(),
	// 									 problem_type->num_possible_actions() + 1,
	// 									 NUM_STATES);
	// 	memory_cells[s_index]->index = s_index;
	// }
	// vector<ActionNetwork*> action_networks(problem_type->num_possible_actions() + 1);
	// for (int a_index = 0; a_index < problem_type->num_possible_actions() + 1; a_index++) {
	// 	action_networks[a_index] = new ActionNetwork(NUM_STATES);
	// }

	vector<LSTM*> memory_cells(NUM_STATES);
	for (int s_index = 0; s_index < NUM_STATES; s_index++) {
		ifstream memory_cell_save_file;
		memory_cell_save_file.open("saves/memory_cell_" + to_string(s_index) + ".txt");
		memory_cells[s_index] = new LSTM(memory_cell_save_file);
		memory_cell_save_file.close();
		memory_cells[s_index]->index = s_index;
	}
	vector<ActionNetwork*> action_networks(problem_type->num_possible_actions() + 1);
	for (int a_index = 0; a_index < problem_type->num_possible_actions() + 1; a_index++) {
		ifstream action_network_save_file;
		action_network_save_file.open("saves/action_network_" + to_string(a_index) + ".txt");
		action_networks[a_index] = new ActionNetwork(action_network_save_file);
		action_network_save_file.close();
	}

	train_network(samples,
				  memory_cells,
				  action_networks);

	for (int s_index = 0; s_index < 200; s_index++) {
		delete samples[s_index];
	}
	for (int s_index = 0; s_index < NUM_STATES; s_index++) {
		delete memory_cells[s_index];
	}
	for (int a_index = 0; a_index < problem_type->num_possible_actions() + 1; a_index++) {
		delete action_networks[a_index];
	}

	delete problem_type;

	cout << "Done" << endl;
}
