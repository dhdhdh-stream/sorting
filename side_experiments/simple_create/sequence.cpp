#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

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

	int iter_index = 0;

	uniform_int_distribution<int> update_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.2);
	uniform_int_distribution<int> action_distribution(0, 2);
	while (true) {
		vector<int> action_sequence;
		int instance_length = 1 + geometric_distribution(generator);
		for (int l_index = 0; l_index < instance_length; l_index++) {
			int action = action_distribution(generator);
			action_sequence.push_back(action);
		}

		bool found_sequence = false;
		for (int a_index = 0; a_index < (int)action_sequence.size()-2; a_index++) {
			if (action_sequence[a_index] == 0
					&& action_sequence[a_index+1] == 1
					&& action_sequence[a_index+2] == 1) {
				found_sequence = true;
			}
		}

		double target_val;
		if (found_sequence) {
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

		if (iter_index%1000000 == 0) {
			cout << "hmm->average_misguess: " << hmm->average_misguess << endl;
		}

		iter_index++;
	}

	delete hmm;

	cout << "Done" << endl;
}
