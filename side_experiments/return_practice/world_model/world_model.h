#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class PredictWrapper;
class StateNetwork;
class Wrapper;

class WorldModel {
public:
	int num_states;

	std::vector<std::vector<int>> network_inputs;
	std::vector<std::vector<int>> network_outputs;
	std::vector<StateNetwork*> obs_networks;
	std::vector<StateNetwork*> action_networks;

	StateNetwork* final_network;

	int epoch_iter;
	int average_max_update;

	PredictWrapper* predict;

	PredictWrapper* candidate_predict;
	int candidate_iter;

	WorldModel(Wrapper* wrapper);
	WorldModel(std::ifstream& input_file);
	~WorldModel();

	void save(std::ofstream& output_file);
};

#endif /* WORLD_MODEL_H */