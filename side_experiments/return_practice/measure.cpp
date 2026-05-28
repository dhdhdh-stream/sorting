#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "run.h"
#include "simple_branch.h"
#include "wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

const int MEASURE_NUM_ITERS = 4000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimpleBranch();

	string filename;
	Wrapper* wrapper;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}
	wrapper = new Wrapper("saves/",
						  filename);

	double sum_scores = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_NUM_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();

		Run* run = new Run();

		wrapper->init(run);

		while (true) {
			vector<double> obs = problem->get_observations();

			pair<bool,int> next = wrapper->step(obs,
												run);
			if (next.first) {
				break;
			} else {
				problem->perform_action(next.second);
			}
		}

		double target_val = problem->score_result();
		sum_scores += target_val;

		delete run;

		delete problem;
	}

	double score_average = sum_scores / MEASURE_NUM_ITERS;
	cout << "score_average: " << score_average << endl;

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
