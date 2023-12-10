#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "experiment.h"
#include "globals.h"
#include "hidden_state.h"
#include "hmm.h"

using namespace std;

default_random_engine generator;

HMM* hmm;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	hmm = new HMM();
	hmm->init();

	// HiddenState* explore_state = hmm->hidden_states[0];
	// int new_starting_action = 0;
	// // int new_starting_action = 1;
	// HiddenState* new_state = new HiddenState();
	// Experiment* new_experiment = new Experiment(explore_state,
	// 											new_starting_action,
	// 											vector<HiddenState*>{new_state});
	// explore_state->experiment = new_experiment;

	HiddenState* explore_state = hmm->hidden_states[0];
	int experiment_starting_action = 0;
	vector<HiddenState*> experiment_states;
	experiment_states.push_back(new HiddenState());
	experiment_states.push_back(new HiddenState());
	{
		experiment_states[0]->transitions[0] = experiment_states[1];
		// experiment_states[0]->transitions[1] = explore_state;
		// experiment_states[0]->transitions[2] = explore_state;
	}
	Experiment* new_experiment = new Experiment(explore_state,
												experiment_starting_action,
												experiment_states);
	explore_state->experiment = new_experiment;

	// HiddenState* explore_state = hmm->hidden_states[0];
	// int experiment_starting_action = 0;
	// vector<HiddenState*> experiment_states;
	// experiment_states.push_back(new HiddenState());
	// experiment_states.push_back(new HiddenState());
	// experiment_states.push_back(new HiddenState());
	// {
	// 	experiment_states[0]->transitions[0] = experiment_states[1];
	// 	experiment_states[0]->transitions[1] = explore_state;
	// 	experiment_states[0]->transitions[2] = explore_state;
	// }
	// {
	// 	experiment_states[1]->transitions[0] = experiment_states[2];
	// 	experiment_states[1]->transitions[1] = explore_state;
	// 	experiment_states[1]->transitions[2] = explore_state;
	// }
	// Experiment* new_experiment = new Experiment(explore_state,
	// 											experiment_starting_action,
	// 											experiment_states);
	// explore_state->experiment = new_experiment;

	int iter_index = 0;

	uniform_int_distribution<int> update_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, 2);
	while (true) {
		vector<int> action_sequence;
		int instance_length = 1 + geometric_distribution(generator);
		int num_0s = 0;
		for (int l_index = 0; l_index < instance_length; l_index++) {
			int action = action_distribution(generator);
			action_sequence.push_back(action);
			if (action == 0) {
				num_0s++;
			}
		}

		double target_val;
		if (num_0s >= 3) {
			target_val = 1.0;
		} else {
			target_val = -1.0;
		}

		if (iter_index < 10000 || update_distribution(generator) == 0) {
			hmm->update(action_sequence,
						target_val);
		} else {
			hmm->explore(action_sequence,
						 target_val);
		}

		iter_index++;
	}

	delete hmm;

	cout << "Done" << endl;
}
