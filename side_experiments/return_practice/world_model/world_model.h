#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class PredictWrapper;
class StateNetwork;

class WorldModel {
public:
	int num_states;

	std::vector<std::vector<int>> network_inputs;
	std::vector<std::vector<int>> network_outputs;
	std::vector<StateNetwork*> obs_networks;
	std::vector<StateNetwork*> action_networks;

	StateNetwork* curr_final_network;

	int curr_epoch_iter;
	int curr_average_max_update;

	double curr_misguess_average;
	double curr_misguess_variance_average;

	std::vector<double> curr_misguess_average_history;
	std::vector<double> curr_misguess_variance_average_history;

	PredictWrapper* curr_predict;

	PredictWrapper* curr_candidate_predict;
	int curr_candidate_iter;

	StateNetwork* large_obs_network;
	StateNetwork* large_action_network;

	StateNetwork* large_final_network;

	int large_epoch_iter;
	int large_average_max_update;

	double large_misguess_average;
	double large_misguess_variance_average;

	std::vector<double> large_misguess_average_history;
	std::vector<double> large_misguess_variance_average_history;

	PredictWrapper* large_predict;

	PredictWrapper* large_candidate_predict;
	int large_candidate_iter;

	WorldModel(int num_obs,
			   int num_actions);
	WorldModel(std::ifstream& input_file);
	~WorldModel();

	void save(std::ofstream& output_file);
};

#endif /* WORLD_MODEL_H */