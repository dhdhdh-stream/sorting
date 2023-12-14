#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "world_model.h"
#include "world_state.h"

using namespace std;

default_random_engine generator;

WorldModel* world_model;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	int seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	world_model = new WorldModel();
	world_model->init();



	cout << "Done" << endl;
}
