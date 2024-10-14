#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

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

	vector<map<vector<int>, int>> counts(10);

	for (int sample_index = 0; sample_index < 200; sample_index++) {
		Sample sample(sample_index);

		for (int l_index = 0; l_index < 10; l_index++) {
			for (int s_index = 1 + 1 + l_index; s_index < (int)sample.actions.size(); s_index++) {
				vector<int> sequence(1 + l_index);
				for (int i_index = 0; i_index < 1 + l_index; i_index++) {
					sequence[i_index] = sample.actions[s_index - (1 + l_index) + i_index].move;
				}

				map<vector<int>, int>::iterator it = counts[l_index].find(sequence);
				if (it == counts[l_index].end()) {
					counts[l_index][sequence] = 1;
				} else {
					it->second++;
				}
			}
		}
	}

	for (int l_index = 0; l_index < 10; l_index++) {
		cout << "l_index: " << l_index << endl;

		int best_count = 0;
		vector<int> best_sequence;
		for (map<vector<int>, int>::iterator it = counts[l_index].begin();
				it != counts[l_index].end(); it++) {
			if (it->second > best_count) {
				best_count = it->second;
				best_sequence = it->first;
			}
		}

		cout << "best_sequence:";
		for (int i_index = 0; i_index < (int)best_sequence.size(); i_index++) {
			cout << " " << best_sequence[i_index];
		}
		cout << endl;
		cout << "best_count: " << best_count << endl;
	}

	delete problem_type;

	cout << "Done" << endl;
}
