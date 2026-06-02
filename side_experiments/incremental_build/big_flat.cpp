#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "network.h"
#include "test_indirect.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_STATE = 40;

#if defined(MDEBUG) && MDEBUG
const int SAMPLES_PER_EPOCH = 40;
#else
const int SAMPLES_PER_EPOCH = 10000;
#endif /* MDEBUG */

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeTestIndirect();

	Wrapper* wrapper = new Wrapper(problem_type);

	vector<int> new_network_input_output;
	for (int i_index = 0; i_index < NUM_STATE; i_index++) {
		new_network_input_output.push_back(i_index);
	}

	wrapper->world_model->num_states += NUM_STATE;

	Network* new_obs_network = new Network(NUM_STATE + wrapper->num_obs, NUM_STATE);
	wrapper->world_model->obs_network_inputs.push_back(new_network_input_output);
	wrapper->world_model->obs_network_outputs.push_back(new_network_input_output);
	wrapper->world_model->obs_networks.push_back(new_obs_network);

	Network* new_action_network = new Network(NUM_STATE + wrapper->num_obs, NUM_STATE);
	wrapper->world_model->action_network_inputs.push_back(new_network_input_output);
	wrapper->world_model->action_network_outputs.push_back(new_network_input_output);
	wrapper->world_model->action_networks.push_back(new_action_network);

	delete wrapper->world_model->score_network;
	wrapper->world_model->score_network = new Network(NUM_STATE, 1);

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);
	while (true) {
		double sum_error = 0.0;
		for (int sample_index = 0; sample_index < SAMPLES_PER_EPOCH; sample_index++) {
			Problem* problem = problem_type->get_problem();

			vector<vector<double>> curr_obs;
			vector<int> curr_actions;

			curr_obs.push_back(problem->get_observations());

			int num_actions = num_actions_distribution(generator);
			for (int a_index = 0; a_index < num_actions; a_index++) {
				int action = action_distribution(generator);

				problem->perform_action(action);
				curr_actions.push_back(action);

				curr_obs.push_back(problem->get_observations());
			}

			// double error;
			update_world_model_helper(curr_obs,
									  curr_actions,
									  problem->score_result(),
									  wrapper->world_model,
									  wrapper);
			// sum_error += error;

			delete problem;
		}

		cout << "sum_error: " << sum_error << endl;
		measure_test(wrapper);
	}

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
