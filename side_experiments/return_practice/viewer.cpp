#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
#include "predict_wrapper.h"
#include "run.h"
#include "solution_helpers.h"
#include "state_network.h"
#include "test_indirect.h"
#include "world_model.h"
#include "world_model_helpers.h"
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

	ProblemType* problem_type = new TypeTestIndirect();

	string filename;
	Wrapper* wrapper;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "main.txt";
	}
	wrapper = new Wrapper("saves/",
						  filename);

	WorldModel* world_model = wrapper->curr_model;

	// temp
	cout << "world_model->curr_predict->misguess_average: " << world_model->curr_predict->misguess_average << endl;

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);

	Problem* problem = problem_type->get_problem();

	vector<double> state(world_model->num_states, 0.0);

	{
		vector<double> start_state = state;

		vector<double> obs = problem->get_observations();

		cout << "obs:";
		for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
			cout << " " << obs[o_index];
		}
		cout << endl;

		obs_helper(obs,
				   state,
				   wrapper);

		cout << "state:";
		for (int s_index = 0; s_index < (int)state.size(); s_index++) {
			cout << " " << state[s_index];
		}
		cout << endl;

		for (int predict_index = 0; predict_index < 10; predict_index++) {
			cout << "predict_index: " << predict_index << endl;
			vector<double> predict_state = start_state;
			predict_helper(predict_state,
						   wrapper);
			cout << "predict_state:";
			for (int s_index = 0; s_index < (int)predict_state.size(); s_index++) {
				cout << " " << predict_state[s_index];
			}
			cout << endl;
		}

		cout << endl;
	}

	int num_actions = num_actions_distribution(generator);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		int action = action_distribution(generator);
		cout << "action: " << action << endl;

		problem->perform_action(action);

		action_helper(action,
					  state,
					  wrapper);

		cout << "state:";
		for (int s_index = 0; s_index < (int)state.size(); s_index++) {
			cout << " " << state[s_index];
		}
		cout << endl;

		vector<double> start_state = state;

		vector<double> obs = problem->get_observations();

		cout << "obs:";
		for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
			cout << " " << obs[o_index];
		}
		cout << endl;

		obs_helper(obs,
				   state,
				   wrapper);

		cout << "state:";
		for (int s_index = 0; s_index < (int)state.size(); s_index++) {
			cout << " " << state[s_index];
		}
		cout << endl;

		for (int predict_index = 0; predict_index < 10; predict_index++) {
			cout << "predict_index: " << predict_index << endl;
			vector<double> predict_state = start_state;
			predict_helper(predict_state,
						   wrapper);
			cout << "predict_state:";
			for (int s_index = 0; s_index < (int)predict_state.size(); s_index++) {
				cout << " " << predict_state[s_index];
			}
			cout << endl;
		}

		cout << endl;
	}

	world_model->final_network->activate(state);
	double predicted = world_model->final_network->output->acti_vals[0];
	cout << "predicted: " << predicted << endl;

	double target_val = problem->score_result();
	cout << "target_val: " << target_val << endl;

	delete problem;

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
