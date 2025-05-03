#include <iostream>

#include "globals.h"
#include "run_helper.h"
#include "two_dimensional.h"
#include "world.h"
#include "world_model_helpers.h"

using namespace std;

default_random_engine generator;

ProblemType* problem_type;
World* world;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	problem_type = new TypeTwoDimensional();

	world = new World();

	Problem* problem = problem_type->get_problem();

	RunHelper run_helper;

	world->init();

	find_initial_pattern(problem,
						 run_helper);

	world->save("saves/", "main.txt");

	// world->load("saves/", "main.txt");



	ofstream display_file;
	display_file.open("saves/display.txt");
	world->save_for_display(display_file);
	display_file.close();

	delete problem;

	delete problem_type;
	delete world;

	cout << "Done" << endl;
}
