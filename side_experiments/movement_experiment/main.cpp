// TODO: extremely simple predict network
// TODO: predict location change

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "simple.h"
#include "world_model.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimple();

	WorldModel* world_model = new WorldModel();

	geometric_distribution<int> num_actions_distribution(0.3);
	uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
	uniform_int_distribution<int> add_uncertainty_distribution(0, 1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		Problem* simple = problem_type->get_problem();

		WorldModelInstance world_model_instance(world_model);

		bool is_uncertain = add_uncertainty_distribution(generator) == 0;

		{
			vector<double> obs = simple->get_observations();
			world_model_instance.train_init(obs);
		}

		int num_actions = 1 + num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_actions; a_index++) {
			int action = action_distribution(generator);
			simple->perform_action(action);
			vector<double> obs = simple->get_observations();
			world_model_instance.train_step(action,
											obs,
											is_uncertain);
		}

		world_model_instance.train_backprop();

		delete simple;

		if (iter_index % 1000 == 0) {
			Simple* simple = (Simple*)problem_type->get_problem();

			WorldModelInstance world_model_instance(world_model);

			{
				vector<double> obs = simple->get_observations();
				world_model_instance.debug_init(obs);
			}

			int num_actions = 1 + num_actions_distribution(generator);
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);
				simple->perform_action(action);
				cout << "simple->current_index: " << simple->current_index << endl;
				vector<double> obs = simple->get_observations();
				world_model_instance.debug_step(action,
												obs);
			}

			delete simple;
		}
	}

	cout << "Done" << endl;
}
