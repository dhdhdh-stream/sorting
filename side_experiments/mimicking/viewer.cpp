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

	Sample sample(0);

	vector<double> state_vals(NUM_STATES, 0.0);

	for (int step_index = 0; step_index < (int)sample.actions.size(); step_index++) {
		cout << "step_index: " << step_index << endl;

		cout << "actions: " << sample.actions[step_index].move << endl;

		cout << "obs:" << endl;
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << sample.obs[step_index][5 * y_index + x_index] << " ";
			}
			cout << endl;
		}
		cout << "(" << sample.locations[step_index][12][0] << "," << sample.locations[step_index][12][1] << ")" << endl;

		for (int s_index = 0; s_index < NUM_STATES; s_index++) {
			memory_cells[s_index]->activate(sample.obs[step_index],
											sample.actions[step_index].move + 1,
											state_vals);
		}

		for (int s_index = 0; s_index < NUM_STATES; s_index++) {
			state_vals[s_index] = memory_cells[s_index]->memory_val;
		}

		vector<double> output_state_vals(NUM_STATES);
		for (int s_index = 0; s_index < NUM_STATES; s_index++) {
			output_state_vals[s_index] = memory_cells[s_index]->output;
		}

		for (int a_index = 0; a_index < (int)action_networks.size(); a_index++) {
			action_networks[a_index]->activate(output_state_vals);
		}

		cout << "TERMINATE: " << action_networks[0]->output->acti_vals[0] << endl;
		cout << "UP: " << action_networks[1]->output->acti_vals[0] << endl;
		cout << "RIGHT: " << action_networks[2]->output->acti_vals[0] << endl;
		cout << "DOWN: " << action_networks[3]->output->acti_vals[0] << endl;
		cout << "LEFT: " << action_networks[4]->output->acti_vals[0] << endl;
		cout << "CLICK: " << action_networks[5]->output->acti_vals[0] << endl;
		cout << "FLAG: " << action_networks[6]->output->acti_vals[0] << endl;
		cout << "DOUBLECLICK: " << action_networks[7]->output->acti_vals[0] << endl;

		cout << endl;
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
