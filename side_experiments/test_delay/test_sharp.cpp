#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "run_helpers.h"
#include "world_model.h"
#include "world_state.h"
#include "world_truth.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	WorldModel* curr_model = new WorldModel();

	uniform_real_distribution<double> distribution(0.0, 1.0);
	for (int s_index = 0; s_index < 4; s_index++) {
		WorldState* world_state = new WorldState();
		world_state->parent = curr_model;
		world_state->id = curr_model->states.size();
		curr_model->states.push_back(world_state);

		world_state->average_val = distribution(generator);

		world_state->transitions = vector<vector<pair<WorldState*,double>>>(NUM_ACTIONS);
	}

	{
		curr_model->states[0]->transitions[ACTION_UP].push_back({curr_model->states[2], 0.5});
		curr_model->states[0]->transitions[ACTION_UP].push_back({curr_model->states[0], 0.5});

		curr_model->states[0]->transitions[ACTION_RIGHT].push_back({curr_model->states[1], 0.5});
		curr_model->states[0]->transitions[ACTION_RIGHT].push_back({curr_model->states[0], 0.5});

		curr_model->states[0]->transitions[ACTION_DOWN].push_back({curr_model->states[0], 1.0});

		curr_model->states[0]->transitions[ACTION_LEFT].push_back({curr_model->states[0], 1.0});
	}

	{
		curr_model->states[1]->transitions[ACTION_UP].push_back({curr_model->states[3], 0.5});
		curr_model->states[1]->transitions[ACTION_UP].push_back({curr_model->states[1], 0.5});

		curr_model->states[1]->transitions[ACTION_RIGHT].push_back({curr_model->states[1], 1.0});

		curr_model->states[1]->transitions[ACTION_DOWN].push_back({curr_model->states[1], 1.0});

		curr_model->states[1]->transitions[ACTION_LEFT].push_back({curr_model->states[0], 0.5});
		curr_model->states[1]->transitions[ACTION_LEFT].push_back({curr_model->states[1], 0.5});
	}

	{
		curr_model->states[2]->transitions[ACTION_UP].push_back({curr_model->states[2], 1.0});

		curr_model->states[2]->transitions[ACTION_RIGHT].push_back({curr_model->states[3], 0.5});
		curr_model->states[2]->transitions[ACTION_RIGHT].push_back({curr_model->states[2], 0.5});

		curr_model->states[2]->transitions[ACTION_DOWN].push_back({curr_model->states[0], 0.5});
		curr_model->states[2]->transitions[ACTION_DOWN].push_back({curr_model->states[2], 0.5});

		curr_model->states[2]->transitions[ACTION_LEFT].push_back({curr_model->states[2], 1.0});
	}

	{
		curr_model->states[3]->transitions[ACTION_UP].push_back({curr_model->states[3], 1.0});

		curr_model->states[3]->transitions[ACTION_RIGHT].push_back({curr_model->states[3], 1.0});

		curr_model->states[3]->transitions[ACTION_DOWN].push_back({curr_model->states[1], 0.5});
		curr_model->states[3]->transitions[ACTION_DOWN].push_back({curr_model->states[3], 0.5});

		curr_model->states[3]->transitions[ACTION_LEFT].push_back({curr_model->states[2], 0.5});
		curr_model->states[3]->transitions[ACTION_LEFT].push_back({curr_model->states[3], 0.5});
	}

	curr_model->starting_likelihood = vector<double>(4, 0.25);

	train_model(curr_model);
	train_model(curr_model);
	train_model(curr_model);
	train_model(curr_model);
	train_model(curr_model);

	{
		ofstream output_file;
		output_file.open("display.txt");
		curr_model->save_for_display(output_file);
		output_file.close();
	}

	cout << "Done" << endl;
}
