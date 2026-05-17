#include "world_model_helpers.h"

#include <iostream>

#include "world_model.h"
#include "world_model_wrapper.h"

using namespace std;

void train_helper(WorldModelWrapper* wrapper) {
	// temp
	cout << "train_helper" << endl;

	vector<vector<double>> init_obs_inputs;
	vector<vector<double>> init_action_inputs;
	vector<double> init_target_vals;
	init_data_helper(init_obs_inputs,
					 init_action_inputs,
					 init_target_vals,
					 wrapper);

	vector<Network*> temp_obs_networks;
	vector<double> temp_obs_network_means;
	vector<double> temp_obs_network_diffs;
	vector<Network*> temp_action_networks;
	vector<double> temp_action_network_means;
	vector<double> temp_action_network_diffs;
	init_temp_helper(init_obs_inputs,
					 init_action_inputs,
					 init_target_vals,
					 temp_obs_networks,
					 temp_obs_network_means,
					 temp_obs_network_diffs,
					 temp_action_networks,
					 temp_action_network_means,
					 temp_action_network_diffs);

	Network* new_final_network;
	stabilize_helper(temp_obs_networks,
					 temp_obs_network_means,
					 temp_obs_network_diffs,
					 temp_action_networks,
					 temp_action_network_means,
					 temp_action_network_diffs,
					 new_final_network,
					 wrapper);

	WorldModel* potential_world_model = new WorldModel(wrapper->world_model);
	ramp_helper(temp_obs_networks,
				temp_obs_network_means,
				temp_obs_network_diffs,
				temp_action_networks,
				temp_action_network_means,
				temp_action_network_diffs,
				new_final_network,
				potential_world_model,
				wrapper);

	finalize_helper(potential_world_model,
					wrapper);

	delete wrapper->world_model;
	wrapper->world_model = potential_world_model;

	// temp
	cout << "train_helper done" << endl;
}
