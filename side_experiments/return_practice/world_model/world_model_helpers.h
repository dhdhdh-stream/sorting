#ifndef WORLD_MODEL_HELPERS_H
#define WORLD_MODEL_HELPERS_H

#include <vector>

#include "run.h"

class Network;
class WorldModel;
class WorldModelWrapper;

double predict_helper(std::vector<double>& existing_state,
					  std::vector<int>& potential_return,
					  WorldModelWrapper* wrapper);

void init_run(Run& run,
			  WorldModelWrapper* wrapper);

void explore_obs_step(std::vector<double>& obs,
					  Run& run,
					  WorldModelWrapper* wrapper);

void explore_action_step(Run& run,
						 int& next_action,
						 bool& is_done,
						 WorldModelWrapper* wrapper);

void init_data_helper(std::vector<std::vector<double>>& init_obs_inputs,
					  std::vector<std::vector<double>>& init_action_inputs,
					  std::vector<double>& init_target_vals,
					  WorldModelWrapper* wrapper);
void init_temp_helper(std::vector<std::vector<double>>& init_obs_inputs,
					  std::vector<std::vector<double>>& init_action_inputs,
					  std::vector<double>& init_target_vals,
					  std::vector<Network*>& temp_obs_networks,
					  std::vector<double>& temp_obs_network_means,
					  std::vector<double>& temp_obs_network_diffs,
					  std::vector<Network*>& temp_action_networks,
					  std::vector<double>& temp_action_network_means,
					  std::vector<double>& temp_action_network_diffs);
void stabilize_helper(std::vector<Network*>& temp_obs_networks,
					  std::vector<double>& temp_obs_network_means,
					  std::vector<double>& temp_obs_network_diffs,
					  std::vector<Network*>& temp_action_networks,
					  std::vector<double>& temp_action_network_means,
					  std::vector<double>& temp_action_network_diffs,
					  Network*& new_final_network,
					  WorldModelWrapper* wrapper);
void ramp_helper(std::vector<Network*>& temp_obs_networks,
				 std::vector<double>& temp_obs_network_means,
				 std::vector<double>& temp_obs_network_diffs,
				 std::vector<Network*>& temp_action_networks,
				 std::vector<double>& temp_action_network_means,
				 std::vector<double>& temp_action_network_diffs,
				 Network* new_final_network,
				 WorldModel* potential_world_model,
				 WorldModelWrapper* wrapper);
void finalize_helper(WorldModel* potential_world_model,
					 WorldModelWrapper* wrapper);
void train_helper(WorldModelWrapper* wrapper);

void init_action_final_data_helper(std::vector<std::vector<double>>& init_action_inputs,
								   std::vector<double>& init_target_vals,
								   WorldModelWrapper* wrapper);
void init_obs_state_data_helper(int state_index,
								std::vector<std::vector<double>>& init_obs_inputs,
								std::vector<double>& init_target_vals,
								WorldModelWrapper* wrapper);
void init_temp_helper(std::vector<std::vector<double>>& init_inputs,
					  std::vector<double>& init_target_vals,
					  Network*& temp_network,
					  double& temp_network_mean,
					  double& temp_network_diff);
void stabilize_action_final_helper(Network* temp_action_network,
								   double temp_action_network_mean,
								   double temp_action_network_diff,
								   Network*& new_final_network,
								   WorldModelWrapper* wrapper);
void stabilize_obs_state_helper(Network* temp_obs_network,
								double temp_obs_network_mean,
								double temp_obs_network_diff,
								int state_index,
								Network*& new_obs_existing_network,
								Network*& new_action_existing_network,
								WorldModelWrapper* wrapper);
void ramp_action_final_helper(Network* temp_action_network,
							  double temp_action_network_mean,
							  double temp_action_network_diff,
							  Network* new_final_network,
							  WorldModel* potential_world_model,
							  WorldModelWrapper* wrapper);
void ramp_obs_state_helper(Network* temp_obs_network,
						   double temp_obs_network_mean,
						   double temp_obs_network_diff,
						   int state_index,
						   Network* new_obs_existing_network,
						   Network* new_action_existing_network,
						   WorldModel* potential_world_model,
						   WorldModelWrapper* wrapper);

#endif /* WORLD_MODEL_HELPERS_H */