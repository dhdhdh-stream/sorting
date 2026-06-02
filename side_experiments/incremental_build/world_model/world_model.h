#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class Network;

class WorldModel {
public:
	int num_states;

	std::vector<std::vector<int>> obs_network_inputs;
	std::vector<std::vector<int>> obs_network_outputs;
	std::vector<Network*> obs_networks;

	std::vector<std::vector<int>> action_network_inputs;
	std::vector<std::vector<int>> action_network_outputs;
	std::vector<Network*> action_networks;

	Network* score_network;

	int epoch_iter;
	double average_max_update;

	WorldModel();
	WorldModel(WorldModel* original);
	~WorldModel();
};

#endif /* WORLD_MODEL_H */