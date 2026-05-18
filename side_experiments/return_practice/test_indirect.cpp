#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "constants.h"
#include "run.h"
#include "simple.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "world_model_wrapper.h"

using namespace std;

int seed;

default_random_engine generator;

const int NUM_SAMPLES = 1000;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	ProblemType* problem_type = new TypeSimple();

	string filename;
	WorldModelWrapper* wrapper;
	if (argc > 1) {
		filename = argv[1];
		wrapper = new WorldModelWrapper(
			"saves/",
			filename);
	} else {
		filename = "main.txt";
		wrapper = new WorldModelWrapper(problem_type);
	}

	geometric_distribution<int> num_actions_distribution(0.1);
	uniform_int_distribution<int> action_distribution(0, wrapper->num_actions-1);
	for (int sample_index = 0; sample_index < NUM_SAMPLES; sample_index++) {
		Problem* problem = problem_type->get_problem();

		int num_actions = 1 + num_actions_distribution(generator);
		vector<int> run_actions;
		for (int a_index = 0; a_index < num_actions; a_index++) {
			run_actions.push_back(action_distribution(generator));
		}

		vector<vector<double>> run_obs;
		for (int a_index = 0; a_index < (int)run_actions.size(); a_index++) {
			vector<double> obs = problem->get_observations();
			run_obs.push_back(obs);

			problem->perform_action(run_actions[a_index]);
		}

		vector<double> obs = problem->get_observations();
		run_obs.push_back(obs);

		double target_val = problem->score_result();
		// // temp
		// cout << "target_val: " << target_val << endl;

		wrapper->sample_obs.push_back(run_obs);
		wrapper->sample_actions.push_back(run_actions);
		wrapper->sample_target_vals.push_back(target_val);

		delete problem;
	}

	{
		vector<vector<double>> init_action_inputs;
		vector<double> init_target_vals;
		init_action_final_data_helper(init_action_inputs,
									  init_target_vals,
									  wrapper);

		Network* temp_action_network;
		double temp_action_network_mean;
		double temp_action_network_diff;
		init_temp_helper(init_action_inputs,
						 init_target_vals,
						 temp_action_network,
						 temp_action_network_mean,
						 temp_action_network_diff);

		Network* new_final_network;
		stabilize_action_final_helper(temp_action_network,
									  temp_action_network_mean,
									  temp_action_network_diff,
									  new_final_network,
									  wrapper);

		WorldModel* potential_world_model = new WorldModel(wrapper->world_model);
		ramp_action_final_helper(temp_action_network,
								 temp_action_network_mean,
								 temp_action_network_diff,
								 new_final_network,
								 potential_world_model,
								 wrapper);

		finalize_helper(potential_world_model,
						wrapper);

		delete wrapper->world_model;
		wrapper->world_model = potential_world_model;
	}

	{
		vector<vector<double>> init_obs_inputs;
		vector<double> init_target_vals;
		init_obs_state_data_helper(0,
								   init_obs_inputs,
								   init_target_vals,
								   wrapper);

		Network* temp_obs_network;
		double temp_obs_network_mean;
		double temp_obs_network_diff;
		init_temp_helper(init_obs_inputs,
						 init_target_vals,
						 temp_obs_network,
						 temp_obs_network_mean,
						 temp_obs_network_diff);

		Network* new_obs_existing_network;
		Network* new_action_existing_network;
		stabilize_obs_state_helper(temp_obs_network,
								   temp_obs_network_mean,
								   temp_obs_network_diff,
								   0,
								   new_obs_existing_network,
								   new_action_existing_network,
								   wrapper);

		WorldModel* potential_world_model = new WorldModel(wrapper->world_model);
		ramp_obs_state_helper(temp_obs_network,
							  temp_obs_network_mean,
							  temp_obs_network_diff,
							  0,
							  new_obs_existing_network,
							  new_action_existing_network,
							  potential_world_model,
							  wrapper);

		finalize_helper(potential_world_model,
						wrapper);

		delete wrapper->world_model;
		wrapper->world_model = potential_world_model;
	}

	delete problem_type;
	delete wrapper;

	cout << "Done" << endl;
}
