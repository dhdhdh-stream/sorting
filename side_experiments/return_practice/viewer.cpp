#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "globals.h"
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

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);

	Problem* problem = problem_type->get_problem();

	vector<double> state(wrapper->curr_model->num_states, 0.0);

	vector<double> obs = problem->get_observations();
	obs_helper(obs,
			   state,
			   wrapper);

	cout << "obs:";
	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
		cout << " " << obs[o_index];
	}
	cout << endl;

	cout << "state:";
	for (int s_index = 0; s_index < (int)state.size(); s_index++) {
		cout << " " << state[s_index];
	}
	cout << endl;

	int num_actions = num_actions_distribution(generator);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		int action = action_distribution(generator);
		cout << "action: " << action << endl;

		problem->perform_action(action);

		vector<double> predicted_state = state;

		action_helper(action,
					  state,
					  wrapper);

		cout << "state:";
		for (int s_index = 0; s_index < (int)state.size(); s_index++) {
			cout << " " << state[s_index];
		}
		cout << endl;

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

		predict_helper(action,
					   predicted_state,
					   wrapper);

		cout << "predicted_state:";
		for (int s_index = 0; s_index < (int)predicted_state.size(); s_index++) {
			cout << " " << predicted_state[s_index];
		}
		cout << endl;
	}

	double target_val = problem->score_result();
	cout << "target_val: " << target_val << endl;

	wrapper->curr_model->final_network->activate(state);
	cout << "wrapper->curr_model->final_network->output->acti_vals[0]: " << wrapper->curr_model->final_network->output->acti_vals[0] << endl;

	delete problem;

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
