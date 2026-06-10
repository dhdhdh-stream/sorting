#ifndef WORLD_MODEL_H
#define WORLD_MODEL_H

#include <fstream>
#include <vector>

class PredictWrapper;
class StateNetwork;

class WorldModel {
public:
	int num_states;

	StateNetwork* obs_network;
	StateNetwork* action_network;
	StateNetwork* final_network;
	int epoch_iter;
	double average_max_update;

	double misguess_average;
	double misguess_variance_average;

	PredictWrapper* curr_predict;

	PredictWrapper* candidate_predict;
	int candidate_iter;

	WorldModel(int num_obs,
			   int num_actions);
	WorldModel(WorldModel* original);
	WorldModel(std::ifstream& input_file);
	~WorldModel();

	void add_states();

	void save(std::ofstream& output_file);
};

#endif /* WORLD_MODEL_H */