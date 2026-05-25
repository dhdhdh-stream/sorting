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

	std::vector<std::vector<int>> final_network_inputs;
	std::vector<Network*> final_networks;

	double average_max_update;

	WorldModel();
	WorldModel(WorldModel* original);
	~WorldModel();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
};

#endif /* WORLD_MODEL_H */